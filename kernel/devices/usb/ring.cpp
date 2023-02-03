#include <string.h>

#include "ring.hpp"

void Ring::Initialize(int buffer_size, MemoryManager& memory_manager) {
    buffer_size_ = buffer_size;
    cycle_bit_ = true;
    write_index_ = 0;

    buffer_ = (TRB*) memory_manager.Allocate((sizeof(TRB) * buffer_size_ + 4095) / 4096).value;
    if (!buffer_) return;
    memset(buffer_, 0, sizeof(TRB) * buffer_size_);
}

TRB* Ring::Buffer() const {
    return buffer_;
}

void EventRing::Initialize(int buffer_size, InterrupterRegisterSet* interrupter, MemoryManager& memory_manager) {
    buffer_size_ = buffer_size;
    cycle_bit_ = true;
    interrupter_ = interrupter;

    buffer_ = (TRB*) memory_manager.Allocate((sizeof(TRB) * buffer_size_ + 4095) /  4096).value;
    if (!buffer_) return;
    memset(buffer_, 0, sizeof(TRB) * buffer_size_);

    erst_ = (EventRingSegmentTableEntry*) memory_manager.Allocate((sizeof(EventRingSegmentTableEntry) * 1 + 44095) / 4096).value;
    if (!erst_) return;
    memset(erst_, 0, sizeof(EventRingSegmentTableEntry) * 1);

    erst_[0].bits.ring_segment_base_address = (uint64_t) buffer_;
    erst_[0].bits.ring_segment_size = buffer_size_;

    interrupter_->ERSTSZ = 1;
    ERDPMap erdp = interrupter_->ERDP.Read();
    erdp.bits.ERDP = (uint64_t) (&buffer_[0]) >> 4;
    interrupter_->ERDP.Write(erdp);
    ERSTBAMap erstba = interrupter_->ERSTBA.Read();
    erstba.bits.ERSTBA = (uint64_t) erst_ >> 6;
    interrupter_->ERSTBA.Write(erstba);
}