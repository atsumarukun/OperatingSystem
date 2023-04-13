#include "graphics/console.hpp"

#include <stdio.h>

void Console::Push(const char* string, unsigned int color) {
    while (*string) {
        if (*string == '\n') {
            Newline();
        } else {
            frame_buffer_writer_.WriteOneLetter(coordinate_, *string, color);
            if (window_size_.width - 18 < (coordinate_.x += 8)) {
                Newline();
            }
        }
        string++;
    }
    frame_buffer_writer_.Draw();
}

void Console::Newline() {
    coordinate_.x = 10;
    if (window_size_.height - 16 < (coordinate_.y + 16)) {
        frame_buffer_writer_.FrameBuffer2Buffer({10, 10}, {10, 26}, {window_size_.width - 10, window_size_.height - 26});
        frame_buffer_writer_.DrawRectangle(coordinate_, {window_size_.width - 10, 16}, 0x222222);
    } else {
        coordinate_.y += 16;
    }
}
