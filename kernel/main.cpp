#include "graphics/framebuffer.hpp"
#include "graphics/graphics.hpp"
#include "memory/memorymap.hpp"
#include "memory/segment.hpp"
#include "memory/paging.hpp"
#include "memory/memory_manager.hpp"
#include "devices/pci.hpp"

#include <stdio.h>

alignas(16) uint8_t kernel_stack[1024 * 1024];

char memory_manager_buffer[sizeof(MemoryManager)];

extern "C" void Main(const FrameBuffer& framebuffer_tmp, const MemoryMap& memorymap_tmp) {
    FrameBuffer framebuffer = framebuffer_tmp;
    MemoryMap memorymap = memorymap_tmp;
    FrameBufferWriter frame_buffer_writer(framebuffer);

    SetSegment();
    MakePageTable();
    MemoryManager* memory_manager = new(memory_manager_buffer) MemoryManager(memorymap);

    PCI pci_devices;

    frame_buffer_writer.DrawRectangle({0, 0}, {framebuffer.width, framebuffer.height}, 0xffffff);

    char s[1024];
    for (int i = 0; i < pci_devices.devices.size(); i++) {
        sprintf(s, "%04x", pci_devices.devices[i].vendor_id);
        frame_buffer_writer.WriteString({10, 10 + i * 16}, s, 0x444444);
    }

    while (1) __asm__("hlt");
}