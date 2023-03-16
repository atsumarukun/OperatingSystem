/*
@file graphics.cpp

グラフィックの設定ファイル.
*/

#include "graphics.hpp"

#include <stdio.h>

extern const uint8_t _binary_graphics_font_bin_start;
extern const uint8_t _binary_graphics_font_bin_size;

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

    const uint8_t* GetFont(char character) {
        auto index = 16 * (unsigned int) character;
        if (index >= (uintptr_t) &_binary_graphics_font_bin_size) return nullptr;
        return &_binary_graphics_font_bin_start + index;
    }

}

FrameBufferWriter::FrameBufferWriter(const FrameBuffer& frame_buffer): frame_buffer_{frame_buffer} {
    buffer_.resize(GetPixelBytes(frame_buffer_.pixel_format) * (frame_buffer_.line * frame_buffer_.height));
}

void FrameBufferWriter::DrawRectangle(Coordinate coordinate, Size size, unsigned int color) {
    for (int y = 0; y < size.height; y++) {
        for (int x = 0; x < size.width; x++) {
            WritePixel({coordinate.x + x, coordinate.y + y}, color);
        }
    }
}

void FrameBufferWriter::DrawCircle(Coordinate coordinate, unsigned int radius, unsigned int color) {
    for (int y = 0; y < radius; y++) {
        for (int x = 0; x < radius; x++) {
            if (x*x + y*y <= radius*radius) {
                WritePixel({coordinate.x + x, coordinate.y + y}, color);
                WritePixel({coordinate.x - x, coordinate.y + y}, color);
                WritePixel({coordinate.x + x, coordinate.y - y}, color);
                WritePixel({coordinate.x - x, coordinate.y - y}, color);
            }
        }
    }
}

void FrameBufferWriter::WriteString(Coordinate Coordinate, const char* string, unsigned int color) {
    for (int i = 0; string[i]; i++) {
        WriteOneLetter({Coordinate.x + 8 * i, Coordinate.y}, string[i], color);
    }
}

void FrameBufferWriter::WriteOneLetter(Coordinate coordinate, char character, unsigned int color) {
    const uint8_t* font = GetFont(character);
    if (!font) return;
    for (int dy = 0; dy < 16; dy++) {
        for (int dx = 0; dx < 8; dx++) {
            if ((font[dy] << dx) & 0x80u) WritePixel({coordinate.x + dx, coordinate.y + dy}, color);
        }
    }
}

void FrameBufferWriter::Draw() {
    memcpy(frame_buffer_.base_address, buffer_.data(), buffer_.size());
}

void FrameBufferWriter::FrameBuffer2Buffer(Coordinate destination, Coordinate original, Size size) {
    int pixel_byte = GetPixelBytes(frame_buffer_.pixel_format);
    for (int y = 0; y < size.height; y++) {
        memcpy(buffer_.data() + pixel_byte * (frame_buffer_.line * (destination.y + y) + destination.x), frame_buffer_.base_address + pixel_byte * (frame_buffer_.line * (original.y + y) + original.x), pixel_byte * size.width);
    }
}

void FrameBufferWriter::WritePixel(Coordinate coordinate, unsigned int color) {
    uint8_t* pixel = &buffer_[GetPixelBytes(frame_buffer_.pixel_format) * (frame_buffer_.line * coordinate.y + coordinate.x)];
    pixel[0] = color >> 16;
    pixel[1] = (color >> 8) & 0xff;
    pixel[2] = color & 0xff;
}