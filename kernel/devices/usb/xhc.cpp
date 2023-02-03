/*
@file xhc.cpp

xhcホストコントローラーのファイル.
*/

#include <stdint.h>
#include <stdio.h>

#include "xhc.hpp"

HostController::HostController(uintptr_t mmio_base_address, MemoryManager& memory_manager):
                                            mmio_base_address_{mmio_base_address},
                                            capability_registers_{(CapabilityRegisters*) mmio_base_address_},
                                            operational_registers_{(OperationalRegisters*) (mmio_base_address_ + capability_registers_->CAPLENGTH)},
                                            max_ports_{(uint8_t) capability_registers_->HCSPARAMS1.Read().bits.MaxPorts} {
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

    uint8_t max_slots = capability_registers_->HCSPARAMS1.Read().bits.MaxSlots;
    CONFIGMap config = operational_registers_->CONFIG.Read();
    config.bits.MaxSlotsEn = max_slots;
    operational_registers_->CONFIG.Write(config);
    device_context_pointers_ = (DeviceContext**) memory_manager.Allocate((sizeof(DeviceContext*) * (max_slots + 1) + 4095) / 44096).value;
    if (!device_context_pointers_) return;
    for (int i = 0; i < max_slots + 1; i++) {
        device_context_pointers_[i] = nullptr;
    }
    DCBAAPMap dcbaap = operational_registers_->DCBAAP.Read();
    dcbaap.bits.DCBAAP = (uint64_t) device_context_pointers_ >> 6;
    operational_registers_->DCBAAP.Write(dcbaap);

    cr_.Initialize(32, memory_manager);
    CRCRMap crcr = operational_registers_->CRCR.Read();
    crcr.bits.RCS = 1;
    crcr.bits.CS = 0;
    crcr.bits.CA = 0;
    crcr.bits.CRP = (uint64_t) cr_.Buffer() >> 6;
    operational_registers_->CRCR.Write(crcr);
    InterrupterRegisterSet* interrupter = &InterrupterRegisterSets()[0];
    er_.Initialize(32, interrupter, memory_manager);

    interrupter->IMAN = 0x3u;
    usbcmd = operational_registers_->USBCMD.Read();
    usbcmd.bits.INTE = 1;
    operational_registers_->USBCMD.Write(usbcmd);

    usbcmd = operational_registers_->USBCMD.Read();
    usbcmd.bits.RS = 1;
    operational_registers_->USBCMD.Write(usbcmd);
    while (operational_registers_->USBSTS.Read().bits.HCH);
}

Array<InterrupterRegisterSet> HostController::InterrupterRegisterSets() const {
    return Array<InterrupterRegisterSet>(mmio_base_address_ + capability_registers_->RTSOFF + 0x20u, 1024);
}