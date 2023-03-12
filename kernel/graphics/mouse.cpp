#include "mouse.hpp"

Mouse::Mouse(FrameBufferWriter& writer): writer_{writer} {
    writer_.DrawCircle({x, y}, 8, 0x222222);
}

void Mouse::Move(int8_t dx, int8_t dy) {
    writer_.DrawCircle({x, y}, 8, 0xffffff);
    x += dx;
    y += dy;
    writer_.DrawCircle({x, y}, 8, 0x222222);
}