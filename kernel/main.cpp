#include "graphics/framebuffer.hpp"
#include "graphics/graphics.hpp"
#include "memory/memorymap.hpp"
#include "memory/segment.hpp"
#include "memory/paging.hpp"
#include "memory/memory_manager.hpp"

#include <stdio.h>

alignas(16) uint8_t kernel_stack[1024 * 1024];

char memory_manager_buffer[sizeof(MemoryManager)];

extern "C" void Main(const FrameBuffer& framebuffer_tmp, const MemoryMap& memorymap_tmp) {
    FrameBuffer framebuffer = framebuffer_tmp;
    MemoryMap memorymap = memorymap_tmp;
    FrameBufferWriter frame_buffer_writer(framebuffer);

    SetSegment();
    MakePageTable();
    MemoryManager* memory_manager = new(memory_manager_buffer) MemoryManager(memorymap);

    frame_buffer_writer.DrawRectangle({0, 0}, {framebuffer.width, framebuffer.height}, 0xffffff);

    char s[1024];
    sprintf(s, "%lu", memory_manager->memory_bit_map_[0]);
    frame_buffer_writer.WriteString({10, 10}, s, 0x444444);
    sprintf(s, "%lu", memory_manager->Allocate(1).value);
    frame_buffer_writer.WriteString({10, 26}, s, 0x444444);
    sprintf(s, "%lu", memory_manager->memory_bit_map_[0]);
    frame_buffer_writer.WriteString({10, 42}, s, 0x444444);
    memory_manager->Free(0, 1);
    sprintf(s, "%lu", memory_manager->memory_bit_map_[0]);
    frame_buffer_writer.WriteString({10, 58}, s, 0x444444);
    while (1) __asm__("hlt");
}