#pragma once

#include "framebuffer.hpp"

#include <vector>

struct Coordinate{
    int x, y;
};

struct Size {
    unsigned int width, height;
};

class FrameBufferWriter {
    public:
        FrameBufferWriter(const FrameBuffer& frame_buffer);
        void Init();
        void DrawRectangle(Coordinate coordinate, Size size, unsigned int color);
        void DrawCircle(Coordinate coordinate, unsigned int radius, unsigned int color);
        void WriteString(Coordinate coordinate, const char* string, unsigned int color);
        void WriteOneLetter(Coordinate Coordinate, char character, unsigned int color);
        void Draw();
        void* Buffer() { return buffer_.data(); }
        void FrameBuffer2Buffer(Coordinate original, Coordinate destination, Size size);

    private:
        const FrameBuffer& frame_buffer_;
        std::vector<uint8_t> buffer_{};
        void WritePixel(Coordinate Coordinate, unsigned int color);
};