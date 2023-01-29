#include "graphics/framebuffer.hpp"
#include "graphics/graphics.hpp"

#include <stdio.h>

extern "C" void Main(const FrameBuffer& frame_buffer_tmp) {
    FrameBuffer frame_buffer = frame_buffer_tmp;
    FrameBufferWriter frame_buffer_writer(frame_buffer);
    frame_buffer_writer.DrawRectangle({0, 0}, {frame_buffer.width, frame_buffer.height}, 0xffffff);

    char s[1024];
    for (int i = 0; i < 5; i++) {
        sprintf(s, "Test: %d", i);
        frame_buffer_writer.WriteString({10, 10 + i * 16}, s, 0x444444);
    }
    while (1) __asm__("hlt");
}