#include "graphics/framebuffer.hpp"
#include "graphics/graphics.hpp"
#include "memory/memorymap.hpp"
#include "memory/segment.hpp"
#include "memory/paging.hpp"
#include "memory/memory_manager.hpp"
#include "devices/pci.hpp"
#include "devices/usb/xhc.hpp"

// #include <stdio.h>

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
    uintptr_t xhc_mmio_base_address = pci_devices.GetXhcMmioBaseAddress();

    frame_buffer_writer.DrawRectangle({0, 0}, {framebuffer.width, framebuffer.height}, 0xffffff);
    HostController xhc(xhc_mmio_base_address, *memory_manager);

    char s[1024];
    sprintf(s, "%s", "Hello OS.");
    frame_buffer_writer.WriteString({10, 10}, s, 0x444444);

    while (1) __asm__("hlt");
}