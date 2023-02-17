/*
@file xhc.cpp

xhcホストコントローラーのファイル.
*/

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "xhc.hpp"
#include "trb.hpp"

int n = 0;

HostController::HostController(uintptr_t mmio_base_address, MemoryManager& memory_manager, FrameBufferWriter& frame_buffer_writer):
                                            mmio_base_address_{mmio_base_address},
                                            capability_registers_{(CapabilityRegisters*) mmio_base_address_},
                                            operational_registers_{(OperationalRegisters*) (mmio_base_address_ + capability_registers_->CAPLENGTH)},
                                            max_ports_{(uint8_t) capability_registers_->HCSPARAMS1.Read().bits.MaxPorts},
                                            memory_manager_{memory_manager} {
    USBCMDMap usbcmd = operational_registers_->USBCMD.Read();
    if (!operational_registers_->USBSTS.Read().bits.HCH) {
        usbcmd.bits.RS = 0;
        operational_registers_->USBCMD.Write(usbcmd);
        while (!operational_registers_->USBSTS.Read().bits.HCH);
    }

    usbcmd = operational_registers_->USBCMD.Read();
    usbcmd.bits.HCRST = 1;
    operational_registers_->USBCMD.Write(usbcmd);
    while (operational_registers_->USBCMD.Read().bits.HCRST);
    while (operational_registers_->USBSTS.Read().bits.CNR);

    uint8_t max_slots_ = capability_registers_->HCSPARAMS1.Read().bits.MaxSlots;
    CONFIGMap config = operational_registers_->CONFIG.Read();
    config.bits.MaxSlotsEn = max_slots_;
    operational_registers_->CONFIG.Write(config);
    device_pointers_ = (USBDevice*) memory_manager_.Allocate((sizeof(USBDevice) * (max_slots_ + 1) + 4095) / 4096).value;
    device_context_pointers_ = (DeviceContext**) memory_manager_.Allocate((sizeof(DeviceContext*) * (max_slots_ + 1) + 4095) / 4096).value;

    DCBAAPMap dcbaap{};
    dcbaap.bits.DCBAAP = (uint64_t) device_context_pointers_ >> 6;
    operational_registers_->DCBAAP.Write(dcbaap);

    cr_.Initialize(32, memory_manager_);
    CRCRMap crcr = operational_registers_->CRCR.Read();
    crcr.bits.RCS = 1;
    crcr.bits.CS = 0;
    crcr.bits.CA = 0;
    crcr.bits.CRP = (uint64_t) cr_.Buffer() >> 6;
    operational_registers_->CRCR.Write(crcr);
    InterrupterRegisterSet* interrupter = &InterrupterRegisterSets()[0];
    er_.Initialize(32, interrupter, memory_manager_);

    interrupter->IMAN &= 0x3u;
    usbcmd = operational_registers_->USBCMD.Read();
    usbcmd.bits.INTE = 1;
    operational_registers_->USBCMD.Write(usbcmd);

    usbcmd = operational_registers_->USBCMD.Read();
    usbcmd.bits.RS = 1;
    operational_registers_->USBCMD.Write(usbcmd);
    while (operational_registers_->USBSTS.Read().bits.HCH);
}

void HostController::ResetPorts() {
    for (int i = 0; i < max_ports_; i++) {
        Port port(i, PortRegisterSets()[i]);
        if (port.IsConnected()) {
            ResetPort(port);
        }
    }
}

void HostController::ProcessEvent(FrameBufferWriter& frame_buffer_writer) {
    if (!er_.HasFront()) return;
    TRB* trb = er_.Front();
    if (PortStatusChangeEventTRB* event_trb = TrbTypeCast<PortStatusChangeEventTRB>(trb)) {
        OnEvent(event_trb);
    } else if (CommandCompletionEventTRB* event_trb = TrbTypeCast<CommandCompletionEventTRB>(trb)) {
        OnEvent(event_trb, frame_buffer_writer);
    } else if (TramsferEventTRB* event_trb = TrbTypeCast<TramsferEventTRB>(trb)) {
        char s[1024];
        sprintf(s, "%d", 3);
        frame_buffer_writer.WriteString({10, 26}, s, 0x444444);
    }
    er_.Pop();
    return;
}

Array<USBDevice> HostController::USBDevices() const {
    return Array<USBDevice>((uintptr_t) device_pointers_, max_slots_ + 1);
}

Array<InterrupterRegisterSet> HostController::InterrupterRegisterSets() const {
    return Array<InterrupterRegisterSet>(mmio_base_address_ + capability_registers_->RTSOFF + 0x20u, 1024);
}

Array<PortRegisterSet> HostController::PortRegisterSets() const {
    return Array<PortRegisterSet>((uintptr_t) operational_registers_ + 0x400u, max_ports_);
}


Array<DoorbellRegister> HostController::DoorbellRegisters() const {
    return Array<DoorbellRegister>(mmio_base_address_ + (capability_registers_->DBOFF.Read().bits.DAO << 2), 256);
}

void HostController::ResetPort(Port port) {
    if (!port.IsConnected()) return;

    if (!setting_port_) {
        setting_port_ = port.ID();
        port.Reset();
    } else {
        waiting_set_ports.push_back(port.ID());
    }
}

void HostController::AllocateSlot(Port port) {
    if (port.IsPortResetChanged() && port.IsEnabled()) {
        port.ClearPortResetChange();
        EnableSlotCommandTRB cmd{};
        cr_.Push(cmd.data);
        DoorbellRegisters()[0].Ring(0);
    }
}

void HostController::InitializeSlotContext(SlotContextMap* context, Port* port) {
    context->bits.route_string = 0;
    context->bits.root_hub_port_number = port->ID() + 1;
    context->bits.context_entries = 1;
    context->bits.speed = port->Speed();
}

void HostController::InitializeEP0Context(EndpointContextMap& context, Ring* ring, uint16_t max_packet_size) {
    context.bits.EPType = 4;
    context.bits.max_packet_size = max_packet_size;
    context.bits.max_burst_size = 0;
    context.bits.tr_dequeue_pointer = (uint64_t) ring->Buffer() << 4;
    context.bits.DCS = 1;
    context.bits.interval = 0;
    context.bits.MaxPStreams = 0;
    context.bits.mult = 0;
    context.bits.CErr = 3;
}

uint16_t DetermineMaxPacketSizeForControlPipe(uint8_t slot_speed) {
    switch (slot_speed) {
        case 4:
            return 512;
        case 3:
            return 64;
        default:
            return 8;
    }
}

void HostController::AddressDevice(uint8_t port_id, uint8_t slot_id) {
    USBDevice* device = new(&USBDevices()[slot_id]) USBDevice(slot_id, &DoorbellRegisters()[slot_id]);
    memset(&device->InputContext()->InputControlContext, 0, sizeof(InputControlContextMap));

    const DeviceContextIndex ep0_dci = DeviceContextIndex(0, false);
    SlotContextMap* slot_context = device->InputContext()->EnableSlotContext();
    EndpointContextMap* ep0_context = device->InputContext()->EnableEndpoint(ep0_dci);

    Port port(port_id, PortRegisterSets()[port_id]);
    InitializeSlotContext(slot_context, &port);

    InitializeEP0Context(*ep0_context, device->AllocateTransferRing(ep0_dci, 32, memory_manager_), DetermineMaxPacketSizeForControlPipe(slot_context->bits.speed));

    device_context_pointers_[slot_id] = device->DeviceContext();

    AddressDeviceCommandTRB cmd{device->InputContext(), slot_id};
    cr_.Push(cmd.data);
    DoorbellRegisters()[0].Ring(0);
}

void HostController::OnEvent(PortStatusChangeEventTRB* trb) {
    uint8_t port_id = (uint8_t) trb->bits.port_id - 1;
    Port port(port_id, PortRegisterSets()[port_id]);
    if (port_id == setting_port_) {
        AllocateSlot(port);
    } else {
        ResetPort(port);
    }
}

void HostController::OnEvent(CommandCompletionEventTRB* trb, FrameBufferWriter& frame_buffer_writer) {
    uint8_t slot_id = (uint8_t)  trb->bits.slot_id;
    TRB* command_trb_pointer = (TRB*) ((uint64_t) trb->bits.command_trb_pointer << 4);
    uint8_t trb_type = (uint8_t) command_trb_pointer->bits.trb_type;
    switch (trb_type) {
        case EnableSlotCommandTRB::TYPE:
            {
                AddressDevice(setting_port_, slot_id);
                break;
            }
        case AddressDeviceCommandTRB::TYPE:
            {
                USBDevice* device = &USBDevices()[slot_id];
                uint8_t port_id = (uint8_t) device->DeviceContext()->SlotContext.bits.root_hub_port_number - 1;
                if (port_id != setting_port_) return;
                setting_port_ = 0;
                if (waiting_set_ports.size()) {
                    uint8_t port_id = waiting_set_ports.back();
                    Port port(port_id, PortRegisterSets()[port_id]);
                    ResetPort(port);
                    waiting_set_ports.pop_back();
                }
                char s[1024];
                sprintf(s, "%d",port_id);
                frame_buffer_writer.WriteString({10, 10 + 16 * n++}, s, 0x444444);
                break;
            }
        default:
            {
                char s[1024];
                sprintf(s, "%d", 100);
                frame_buffer_writer.WriteString({10, 10}, s, 0x444444);
                break;
            }
    }
}

void HostController::Test(FrameBufferWriter& frame_buffer_writer) {
    while (1) {
        ProcessEvent(frame_buffer_writer);
    }
    // sprintf(s, "%d", 100);
    // frame_buffer_writer.WriteString({10, 10}, s, 0x444444);
}