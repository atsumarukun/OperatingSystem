/*
@file graphics.cpp

グラフィックの設定ファイル.
*/

#include "graphics.hpp"

namespace {
    int GetPixelBytes(enum PixelFormat pixel_format) {
        switch (pixel_format) {
            case RGBResv8bit:
                return 4;
            case BGRResv8bit:
                return 4;
        }
        return -1;
    }
}

void FrameBufferWriter::DrawRectangle(Coordinate coordinate, Size size, unsigned int color) {
    for (int y = 0; y < size.height; y++) {
        for (int x = 0; x < size.width; x++) {
            WritePixel({coordinate.x + x, coordinate.y + y}, color);
        }
    }
}

void FrameBufferWriter::WritePixel(Coordinate coordinate, unsigned int color) {
    uint8_t* pixel = &frame_buffer_.base_address[GetPixelBytes(frame_buffer_.pixel_format) * (frame_buffer_.line * coordinate.y + coordinate.x)];
    pixel[0] = color >> 16;
    pixel[1] = (color >> 8) & 0xff;
    pixel[2] = color & 0xff;
}