#include "device.hpp"

Ring* USBDevice::AllocateTransferRing(DeviceContextIndex index, int buffer_size, MemoryManager& memory_manager) {
    int i = index.value - 1;
    Ring* tr = new(&USBDevice::Rings()[i]) Ring;
    tr->Initialize(buffer_size, memory_manager);
    return tr;
}

Array<Ring> USBDevice::Rings() const {
    return Array<Ring>((uintptr_t) transfer_rings_, 31);
}
