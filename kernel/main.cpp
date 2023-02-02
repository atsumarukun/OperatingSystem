#include "graphics/framebuffer.hpp"
#include "graphics/graphics.hpp"
#include "memory/memorymap.hpp"
#include "memory/segment.hpp"
#include "memory/paging.hpp"

#include <stdio.h>

alignas(16) uint8_t kernel_stack[1024 * 1024];

extern "C" void Main(const FrameBuffer& frame_buffer_tmp, const MemoryMap& memory_map_tmp) {
    FrameBuffer frame_buffer = frame_buffer_tmp;
    MemoryMap memory_map = memory_map_tmp;
    FrameBufferWriter frame_buffer_writer(frame_buffer);

    SetSegment();
    MakePageTable();

    frame_buffer_writer.DrawRectangle({0, 0}, {frame_buffer.width, frame_buffer.height}, 0xffffff);

    char s[1024];
    sprintf(s, "%p", kernel_stack + 1024 * 1024);
    frame_buffer_writer.WriteString({10, 10}, s, 0x444444);
    while (1) __asm__("hlt");
}