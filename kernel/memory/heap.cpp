#include "memory/heap.hpp"

extern "C" caddr_t program_break, program_break_end;
void InitializeHeap(MemoryManager* memory_manager) {
    const int kHeapFrames = 64 * 512;
    const auto heap_start = memory_manager->Allocate(kHeapFrames);

    program_break = (caddr_t) heap_start.value;
    program_break_end = program_break + kHeapFrames * 4096;
}
