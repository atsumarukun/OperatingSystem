#pragma once

#include "registry.hpp"
#include "context.hpp"
#include "ring.hpp"
#include "graphics/graphics.hpp"

class USBDevice {
    public:
        USBDevice(uint8_t slot_id, DoorbellRegister* doorbell_reg): slot_id_{slot_id}, doorbell_reg_{doorbell_reg} {}
        DeviceContext* DeviceContext() { return &device_context_; }
        InputContext* InputContext() { return &input_context_; }
        Ring* AllocateTransferRing(DeviceContextIndex index, int buffer_size, MemoryManager& memory_manager);

    private:
        uint8_t slot_id_;
        DoorbellRegister* doorbell_reg_;
        alignas(64) struct DeviceContext device_context_;
        alignas(64) struct InputContext input_context_;
        Ring transfer_rings_[31];

        Array<Ring> Rings() const;
};