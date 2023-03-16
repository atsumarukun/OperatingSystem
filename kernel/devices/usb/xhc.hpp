#pragma once

#include "registry.hpp"
#include "device.hpp"
#include "context.hpp"
#include "ring.hpp"
#include "port.hpp"
#include "memory/memory_manager.hpp"
#include "graphics/graphics.hpp"

#include <vector>
#include <algorithm>

class HostController {
    public:
        HostController(uintptr_t mmio_base_address, MemoryManager& memory_manager);
        void ResetPorts();
        void Test();

    private:
        const uintptr_t mmio_base_address_;
        CapabilityRegisters* const capability_registers_;
        OperationalRegisters* const operational_registers_;
        uint8_t max_ports_;
        uint8_t max_slots_;
        MemoryManager& memory_manager_;
        USBDevice* device_pointers_;
        DeviceContext** device_context_pointers_;
        Ring cr_;
        EventRing er_;
        uint8_t setting_port_;
        std::vector<uint8_t> waiting_set_ports;

        Array<USBDevice> USBDevices() const;
        Array<InterrupterRegisterSet> InterrupterRegisterSets() const;
        Array<PortRegisterSet> PortRegisterSets() const;
        Array<DoorbellRegister> DoorbellRegisters() const;
        void ResetPort(Port port);
        void AllocateSlot(Port port);
        void InitializeSlotContext(SlotContextMap* context, Port* port);
        void InitializeEP0Context(EndpointContextMap* context, Ring* ring, uint16_t max_package_size);
        void AddressDevice(uint8_t port_id, uint8_t slot_id);
        void ConfigureEndpoints(USBDevice* device);
        void OnEvent(PortStatusChangeEventTRB* trb);
        void OnEvent(CommandCompletionEventTRB* trb);
        void OnEvent(TransferEventTRB* trb);
        void ProcessEvent();
};