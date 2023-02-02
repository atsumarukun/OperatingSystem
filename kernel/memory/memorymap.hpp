#pragma once

#include <stdint.h>

typedef struct {
    unsigned long long map_buf_size;
    void* map_buf;
    unsigned long long map_size;
    unsigned long long map_key;
    unsigned long long descriptor_size;
    uint32_t descriptor_version;
} MemoryMap;

#ifdef __cplusplus

enum class MemoryType: uint32_t {
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServiceCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiPersistentMemory,
};

typedef struct {
    enum MemoryType type;
    uintptr_t physical_address;
    uintptr_t virtual_address;
    uint64_t number_of_pages;
    uint64_t attribute;
} MemoryDescriptor;

inline bool IsAvailableMemoryType(MemoryType type) {
    return type == MemoryType::EfiLoaderCode       ||
           type == MemoryType::EfiLoaderData       ||
           type == MemoryType::EfiBootServicesCode ||
           type == MemoryType::EfiBootServicesData ||
           type == MemoryType::EfiConventionalMemory;
}

#endif