#pragma once

#include <stdint.h>

#include "trb.hpp"
#include "registry.hpp"
#include "../../memory/memory_manager.hpp"

union EventRingSegmentTableEntry {
    uint32_t data[4];

    struct {
        uint64_t ring_segment_base_address;

        uint32_t ring_segment_size: 16;
        uint32_t : 16;

        uint32_t : 32;
    } __attribute__((packed)) bits;
};

class Ring {
    public:
        void Initialize(int buffer_size, MemoryManager& memory_manager);
        TRB* Buffer() const;

        TRB* Push(uint32_t trb[4]);

    private:
        TRB* buffer_;
        int buffer_size_;
        bool cycle_bit_;
        unsigned int write_index_;
};

class EventRing {
    public:
        void Initialize(int buffer_size, InterrupterRegisterSet* interrupter, MemoryManager& memory_manager);
        TRB* Front() const;
        bool HasFront() const;
        void Pop();

    private:
        TRB* buffer_;
        int buffer_size_;
        bool cycle_bit_;
        EventRingSegmentTableEntry* erst_;
        InterrupterRegisterSet* interrupter_;

        TRB* ReadDequeuePointer() const {
            return (TRB*) (interrupter_->ERDP.Read().bits.ERDP << 4);
        }

        void WriteDequeuePointer(TRB* dequeue_pointer) {
            ERDPMap erdp = interrupter_->ERDP.Read();
            erdp.bits.ERDP = (uint64_t) dequeue_pointer >> 4;
            interrupter_->ERDP.Write(erdp);
        }
};