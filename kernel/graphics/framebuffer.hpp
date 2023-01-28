#pragma once

#include <stdint.h>

enum PixelFormat {
    RGBResv8bit,
    BGRResv8bit
};

typedef struct {
    uint8_t* base_address;
    uint32_t line;
    uint32_t width;
    uint32_t height;
    enum PixelFormat pixel_format;
} FrameBuffer;