#include "graphics/framebuffer.hpp"
#include "graphics/graphics.hpp"

extern "C" void Main(const FrameBuffer& frame_buffer_tmp) {
    FrameBuffer frame_buffer = frame_buffer_tmp;
    FrameBufferWriter frame_buffer_writer(frame_buffer);
    frame_buffer_writer.DrawRectangle({0, 0}, {frame_buffer.width, frame_buffer.height}, 0xffffff);
    while (1) __asm__("hlt");
}