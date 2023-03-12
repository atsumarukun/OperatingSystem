#pragma once

#include "graphics.hpp"

class Mouse {
    public:
        Mouse(FrameBufferWriter& writer);
        void Move(int8_t displacement_x, int8_t displacement_y);

    private:
        FrameBufferWriter& writer_;
        int x = 100;
        int y = 100;
};