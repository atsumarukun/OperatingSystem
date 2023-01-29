#pragma once

#include <stdint.h>

enum SegmentDescriptorType{
    //DataSegmentType
    ReadOnly = 0x0,
    ReadOnlyAccessed = 0x1,
    ReadWrite = 0x2,
    ReadWriteAccessed = 0x3,
    ExpandDownReadOnly = 0x4,
    ExpandDownReadOnlyAccessed = 0x5,
    ExpandDownReadWrite = 0x6,
    ExpandDownReadWriteAccessed = 0x7,
    //CodeSegmentType
    ExecuteOnly = 0x8,
    ExecuteOnlyAccessed = 0x9,
    ExecuteReadable = 0xa,
    ExecuteReadableAccessed = 0xb,
    ConformingExecuteOnly = 0xc,
    ConformingExecuteOnlyAccessed = 0xd,
    ConformingExecuteReadable = 0xe,
    ConformingExecuteReadableAccessed = 0xf
};

union SegmentDescriptor {
    uint64_t data;
    struct {
        uint64_t segment_limit_low: 16;
        uint64_t base_address_low: 16;
        uint64_t base_address_middle: 8;
        SegmentDescriptorType type: 4;
        uint64_t system_segment: 1;
        uint64_t descriptor_privilege_level: 2;
        uint64_t present: 1;
        uint64_t segment_limit_high: 4;
        uint64_t available: 1;
        uint64_t long_mode: 1;
        uint64_t default_operation_size: 1;
        uint64_t granularity: 1;
        uint64_t base_address_high: 8;
    } __attribute__((packed)) bits;
} __attribute__((packed));

struct GDTR {
    uint16_t limit;
    uintptr_t address;
} __attribute__((packed));

void SetSegment();