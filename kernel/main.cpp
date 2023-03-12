#include "graphics/framebuffer.hpp"
#include "graphics/graphics.hpp"
#include "graphics/mouse.hpp"
#include "memory/memorymap.hpp"
#include "memory/segment.hpp"
#include "memory/paging.hpp"
#include "memory/memory_manager.hpp"
#include "devices/pci.hpp"
#include "devices/usb/xhc.hpp"
#include "devices/usb/mouse.hpp"

#include <stdio.h>

alignas(16) uint8_t kernel_stack[1024 * 1024];

char memory_manager_buffer[sizeof(MemoryManager)];
char mouse_buffer[sizeof(Mouse)];
Mouse* mouse;

void MouseObserver(int8_t displacement_x, int8_t displacement_y) {
    mouse->Move(displacement_x, displacement_y);
}

extern "C" void Main(const FrameBuffer& framebuffer_tmp, const MemoryMap& memorymap_tmp) {
    FrameBuffer framebuffer = framebuffer_tmp;
    MemoryMap memorymap = memorymap_tmp;
    FrameBufferWriter frame_buffer_writer(framebuffer);

    SetSegment();
    MakePageTable();
    MemoryManager* memory_manager = new(memory_manager_buffer) MemoryManager(memorymap);
    memory_manager->Allocate(1);

    PCI pci_devices;
    uintptr_t xhc_mmio_base_address = pci_devices.GetXhcMmioBaseAddress();

    frame_buffer_writer.DrawRectangle({0, 0}, {framebuffer.width, framebuffer.height}, 0xffffff);
    HostController xhc(xhc_mmio_base_address, *memory_manager, frame_buffer_writer);
    mouse = new(mouse_buffer) Mouse(frame_buffer_writer);
    MouseDriver::default_observer = MouseObserver;
    xhc.ResetPorts();
    xhc.Test();

    // char s[1024];
    // sprintf(s, "%s", "Hello OS.");
    // frame_buffer_writer.WriteString({10, 10}, s, 0x444444);

    while (1) __asm__("hlt");
}