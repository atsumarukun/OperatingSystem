/*
@file segment.cpp

セグメントの設定ファイル.
*/

#include <array>

#include "segment.hpp"
#include "../registry/registry.h"

using namespace std;

GDTR gdtr;

namespace {
    array<SegmentDescriptor, 2> gdt;

    void SetCodeSegment(SegmentDescriptor& descriptor, enum SegmentDescriptorType type, uint8_t descriptor_privilege_level) {
        descriptor.data = 0;

        descriptor.bits.type = type;
        descriptor.bits.system_segment = 1;
        descriptor.bits.descriptor_privilege_level = descriptor_privilege_level;
        descriptor.bits.present = 1;
        descriptor.bits.long_mode = 1;
        descriptor.bits.default_operation_size = 0;
    }

    void SetGlobalDescriptorTable() {
        gdt[0].data = 0;
        SetCodeSegment(gdt[1], ExecuteReadable, 0);
        gdtr.limit = sizeof(gdt) - 1;
        gdtr.address = (uintptr_t) &gdt[0];
        __asm__ ("lgdt gdtr");
    }
}

void SetSegment() {
    SetGlobalDescriptorTable();
    SetCS(1 << 3);
    SetSegmentRegistor(0);
}