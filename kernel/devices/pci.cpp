#include "pci.hpp"

namespace {
    uint16_t CONFIG_ADDRESS_REGISTER = 0x0cf8;
    uint16_t CONFIG_DATA_REGISTER    = 0x0cfc;
}

PCI::PCI() {
    uint8_t header_type = ReadHeaderType(0, 0, 0);
    uint8_t function_num = (header_type & 0x80)? 8: 1;
    for (uint8_t function = 0; function < function_num; function++) {
        if (ReadVendorId(0, 0, function) == 0xffffu) continue;
        ScanBus(function);
    }
}

WithError<uint64_t> PCI::ReadBar(Device device, uint8_t index) {
    if (5 < index) return {0, MakeError(Error::PciBarIndexOutOfRange)};
    uint32_t bar = ReadConfigAreaRow(device.bus, device.device, device.function, 0x10 + 4 * index);
    if (!(bar >> 2 & 1)) return {bar, MakeError(Error::Success)};

    if (5 < (index + 1)) return {0, MakeError(Error::PciBarIndexOutOfRange)};
    uint64_t bar_high = (uint64_t) ReadConfigAreaRow(device.bus, device.device, device.function, 0x10u + 4 * (index + 1));

    return {bar | bar_high << 32, MakeError(Error::Success)};
}

uint32_t PCI::MakeConfigAddress(uint8_t bus, uint8_t device, uint8_t function, uint8_t off_set) {
    return 1 << 31 | bus << 16 | device << 11 | function << 8 | (off_set & 0xfcu);
}

uint32_t PCI::ReadConfigAreaRow(uint8_t bus, uint8_t device, uint8_t function, uint8_t off_set) {
    uint32_t config_address = MakeConfigAddress(bus, device, function, off_set);
    uint32_t config_area;
    __asm__ volatile("outl %%eax, %%dx":: "a"(config_address), "d"(CONFIG_ADDRESS_REGISTER));
    __asm__ volatile("inl %%dx, %%eax": "=a"(config_area): "d"(CONFIG_DATA_REGISTER));
    return config_area;
}

uint16_t PCI::ReadVendorId(uint8_t bus, uint8_t device, uint8_t function) {
    return ReadConfigAreaRow(bus, device, function, 0x00) & 0xffffu;
}

uint8_t PCI::ReadHeaderType(uint8_t bus, uint8_t device, uint8_t function) {
    return (ReadConfigAreaRow(bus, device, function, 0x0c) >> 16) & 0xffu;
}

ClassCode PCI::ReadClassCode(uint8_t bus, uint8_t device, uint8_t function) {
    uint32_t config_area_row = ReadConfigAreaRow(bus, device, function, 0x08);
    ClassCode class_code;
    class_code.base_class = (config_area_row >> 24) & 0xffu;
    class_code.sub_class = (config_area_row >> 16) & 0xffu;
    class_code.interface = (config_area_row >> 8) & 0xffu;
    return class_code;
}

uint8_t PCI::ReadSecondaryBus(uint8_t bus, uint8_t device, uint8_t function) {
    return ReadConfigAreaRow(bus, device, function, 0x18) >> 8 & 0xffu;
}

void PCI::ScanFunction(uint8_t bus, uint8_t device, uint8_t function) {
    uint16_t vendor_id = ReadVendorId(bus, device, function);
    uint8_t header_type = ReadHeaderType(bus, device, function);
    ClassCode class_code = ReadClassCode(bus, device, function);
    devices.push_back({bus, device, function, header_type, vendor_id, class_code});

    if (class_code.base_class == 0x06u && class_code.sub_class == 0x04u) {
        uint8_t secondary_bus = ReadSecondaryBus(bus, device, function);
        ScanBus(secondary_bus);
    }
}

void PCI::ScanDevice(uint8_t bus, uint8_t device) {
    uint8_t header_type = ReadHeaderType(bus, device, 0);
    uint8_t function_num = (header_type & 0x80)? 8: 1;
    for (uint8_t function = 0; function < function_num; function++) {
        if (ReadVendorId(bus, device, function) == 0xffffu) continue;
        ScanFunction(bus, device, function);
    }
}

void PCI::ScanBus(uint8_t bus) {
    for (uint8_t device = 0; device < 32; device++) {
        if (ReadVendorId(bus, device, 0) == 0xffffu) continue;
        ScanDevice(bus, device);
    }
}