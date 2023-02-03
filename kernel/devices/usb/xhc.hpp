#pragma once

#include "registry.hpp"
#include "context.hpp"
#include "ring.hpp"
#include "memory/memory_manager.hpp"
#include "graphics/graphics.hpp"

class HostController {
    public:
        HostController(uintptr_t mmio_base_address, MemoryManager& memory_manager);

    private:
        const uintptr_t mmio_base_address_;
        CapabilityRegisters* const capability_registers_;
        OperationalRegisters* const operational_registers_;
        uint8_t max_ports_;
        DeviceContext** device_context_pointers_;
        Ring cr_;
        EventRing er_;

        Array<InterrupterRegisterSet> InterrupterRegisterSets() const;
};