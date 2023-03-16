#pragma once

#include "graphics.hpp"

class Console {
    public:
        Console(Size window_size, Size font_size, FrameBufferWriter& frame_buffer_writer): window_size_{window_size}, font_size_{font_size}, frame_buffer_writer_{frame_buffer_writer} {};
        void Push(const char* string, unsigned int color = 0xffffff);

    private:
        Coordinate coordinate_ = {10, 10};
        Size window_size_;
        Size font_size_;
        FrameBufferWriter& frame_buffer_writer_;

        void Newline();
};