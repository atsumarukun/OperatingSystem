#pragma once

#include <array>

#include "memorymap.hpp"
#include "../error/error.hpp"

using namespace std;

namespace {
    constexpr unsigned long long operator""_KB(unsigned long long kb) {
        return kb * 1024;
    }
    constexpr unsigned long long operator""_MB(unsigned long long mb) {
        return mb * 1024_KB;
    }
    constexpr unsigned long long operator""_GB(unsigned long long gb) {
        return gb * 1024_MB;
    }

    static const unsigned int UEFI_FRAME_SIZE = 4_KB;
    static const unsigned int FRAME_SIZE = 4_KB;
    static const unsigned int NUMBER_OF_FRAMES_SUPPORTED = 128_GB / FRAME_SIZE;
    static const unsigned int NUMBER_OF_MAP_LINE_BITS = 8 * sizeof(uint64_t);
}

class MemoryManager {
    public:
        MemoryManager(const MemoryMap& memory_map);
        WithError<uintptr_t> Allocate(size_t number_of_frames);
        void Free(uintptr_t adress, size_t number_of_frames);
        array<uint64_t, NUMBER_OF_FRAMES_SUPPORTED / NUMBER_OF_MAP_LINE_BITS> memory_bit_map_;

    private:
        // array<uint64_t, NUMBER_OF_FRAMES_SUPPORTED / NUMBER_OF_MAP_LINE_BITS> memory_bit_map_;
        unsigned int last_frame_index_ = NUMBER_OF_FRAMES_SUPPORTED;

        void SetIsAllocationBits(unsigned int start_frame_index, size_t number_of_frames, bool is_allocation);
};