#pragma once

#include <cstdint>

template <class T>
T* DescriptorTypeCast(uint8_t* desc_data) {
    if (desc_data[1] == T::TYPE) {
        return reinterpret_cast<T*>(desc_data);
    }
    return nullptr;
}

struct DeviceDescriptor {
    static const uint8_t TYPE = 1;
    uint8_t length;
    uint8_t descriptor_type;
    uint16_t usb_release;
    uint8_t device_class;
    uint8_t device_sub_class;
    uint8_t device_protocol;
    uint8_t max_packet_size;
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t device_release;
    uint8_t manufacturer;
    uint8_t product;
    uint8_t serial_number;
    uint8_t num_configurations;
} __attribute__((packed));

struct ConfigurationDescriptor {
    static const uint8_t TYPE = 2;
    uint8_t length;
    uint8_t descriptor_type;
    uint16_t total_length;
    uint8_t num_interfaces;
    uint8_t configuration_value;
    uint8_t configuration_id;
    uint8_t attributes;
    uint8_t max_power;
} __attribute__((packed));


  struct InterfaceDescriptor {
    static const uint8_t TYPE = 4;
    uint8_t length;
    uint8_t descriptor_type;
    uint8_t interface_number;
    uint8_t alternate_setting;
    uint8_t num_endpoints;
    uint8_t interface_class;
    uint8_t interface_sub_class;
    uint8_t interface_protocol;
    uint8_t interface_id;
  } __attribute__((packed));

    struct EndpointDescriptor {
    static const uint8_t TYPE = 5;
    uint8_t length;
    uint8_t descriptor_type;
    union {
      uint8_t data;
      struct {
        uint8_t number : 4;
        uint8_t : 3;
        uint8_t dir_in : 1;
      } __attribute__((packed)) bits;
    } endpoint_address;
    union {
      uint8_t data;
      struct {
        uint8_t transfer_type : 2;
        uint8_t sync_type : 2;
        uint8_t usage_type : 2;
        uint8_t : 2;
      } __attribute__((packed)) bits;
    } attributes;
    uint16_t max_packet_size;
    uint8_t interval;
  } __attribute__((packed));