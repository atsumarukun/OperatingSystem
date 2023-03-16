#include "graphics/framebuffer.hpp"
#include "graphics/graphics.hpp"
#include "graphics/mouse.hpp"
#include "graphics/console.hpp"
#include "graphics/logger.hpp"
#include "memory/memorymap.hpp"
#include "memory/segment.hpp"
#include "memory/paging.hpp"
#include "memory/memory_manager.hpp"
#include "memory/heap.hpp"
#include "devices/pci.hpp"
#include "devices/usb/xhc.hpp"
#include "devices/usb/mouse.hpp"

#include <stdio.h>

alignas(16) uint8_t kernel_stack[1024 * 1024];

char memory_manager_buffer[sizeof(MemoryManager)];
char logger_buffer[sizeof(Logger)];
Logger* logger;
char mouse_buffer[sizeof(Mouse)];
Mouse* mouse;

void MouseObserver(int8_t displacement_x, int8_t displacement_y) {
    mouse->Move(displacement_x, displacement_y);
}

extern "C" void Main(const FrameBuffer& frame_buffer_tmp, const MemoryMap& memorymap_tmp) {
    FrameBuffer frame_buffer = frame_buffer_tmp;
    MemoryMap memorymap = memorymap_tmp;

    SetSegment();
    MakePageTable();
    MemoryManager* memory_manager = new(memory_manager_buffer) MemoryManager(memorymap);
    InitializeHeap(memory_manager);
    memory_manager->Allocate(1);

    FrameBufferWriter frame_buffer_writer(frame_buffer);
    frame_buffer_writer.DrawRectangle({0, 0}, {frame_buffer.width, frame_buffer.height}, 0x222222);
    frame_buffer_writer.Draw();

    Console console({frame_buffer.width, frame_buffer.height}, {8, 16}, frame_buffer_writer);
    logger = new(logger_buffer) Logger(console);

    PCI pci_devices;
    uintptr_t xhc_mmio_base_address = pci_devices.GetXhcMmioBaseAddress();

    HostController xhc(xhc_mmio_base_address, *memory_manager, frame_buffer_writer);
    mouse = new(mouse_buffer) Mouse(frame_buffer_writer);
    MouseDriver::default_observer = MouseObserver;
    xhc.ResetPorts();
    xhc.Test();

    while (1) __asm__("hlt");
}