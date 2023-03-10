#pragma once

#include "endpoint.hpp"

union SlotContextMap {
    uint32_t data[8];
    struct {
        uint32_t    route_string: 20;
        uint32_t           speed: 4;
        uint32_t                : 1;
        uint32_t             MTT: 1;
        uint32_t             hub: 1;
        uint32_t context_entries: 5;

        uint32_t     max_exit_latency: 16;
        uint32_t root_hub_port_number: 8;
        uint32_t      number_of_ports: 8;

        uint32_t parent_hub_slot_id: 8;
        uint32_t parent_port_number: 8;
        uint32_t                TTT: 2;
        uint32_t                   : 4;
        uint32_t   interrupt_target: 10;

        uint32_t usb_device_address: 8;
        uint32_t : 19;
        uint32_t slot_state: 5;
    } __attribute__((packed)) bits;
} __attribute__((packed));

union EndpointContextMap {
    uint32_t data[8];
    struct {
        uint32_t          EPState: 3;
        uint32_t                 : 5;
        uint32_t             mult: 2;
        uint32_t      MaxPStreams: 5;
        uint32_t              LSA: 1;
        uint32_t         interval: 8;
        uint32_t MaxESITPayloadHi: 8;

        uint32_t                : 1;
        uint32_t            CErr: 2;
        uint32_t          EPType: 3;
        uint32_t                : 1;
        uint32_t             HID: 1;
        uint32_t  max_burst_size: 8;
        uint32_t max_packet_size: 16;

        uint64_t                DCS: 1;
        uint64_t                   : 3;
        uint64_t tr_dequeue_pointer: 60;

        uint32_t average_trb_length: 16;
        uint32_t   MaxESITPayloadLo: 16;
    } __attribute__((packed)) bits;
} __attribute__((packed));

struct DeviceContextIndex {
    int value;
    DeviceContextIndex(EndpointID endpoint_id) : value{endpoint_id.Address()} {}
    DeviceContextIndex(int ep_num, bool dir_in): value{2 * ep_num + (ep_num == 0? 1: dir_in)} {}
};

struct DeviceContext {
    SlotContextMap SlotContext;
    EndpointContextMap EndpointContext[31];
} __attribute__((packed));

struct InputControlContextMap {
    uint32_t drop_context_flags;
    uint32_t add_context_flags;
    uint32_t reserved1[5];
    uint8_t configuration_value;
    uint8_t interface_number;
    uint8_t alternate_setting;
    uint8_t reserved2;
} __attribute__((packed));

struct InputContext {
    InputControlContextMap InputControlContext;
    SlotContextMap SlotContext;
    EndpointContextMap EndpointContext[31];

    SlotContextMap* EnableSlotContext() {
        InputControlContext.add_context_flags |= 1;
        return &SlotContext;
    }

    EndpointContextMap* EnableEndpoint(DeviceContextIndex dci) {
        InputControlContext.add_context_flags |= 1u << dci.value;
        return &EndpointContext[dci.value - 1];
    }
} __attribute__((packed));