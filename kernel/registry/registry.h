#pragma once

#include <stdint.h>

extern "C" {
    void SetSegmentRegistor(uint16_t value);
    void SetCS(uint16_t cs);
}
