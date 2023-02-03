#pragma once

#include <stdint.h>

template <typename ToType, typename FromeType>
ToType* TrbTypeCast(FromeType* trb) {
    if (ToType::TYPE == trb->bits.trb_type) return (ToType*) trb;
    return nullptr;
}

union TRB {
    uint32_t data[4];
    struct {
        uint64_t parameter;

        uint32_t status;

        uint32_t         cycle_bit: 1;
        uint32_t evaluate_next_trb: 1;
        uint32_t                  : 8;
        uint32_t          trb_type: 6;
        uint32_t           control: 16;
    } __attribute__((packed)) bits;
};

union EnableSlotCommandTRB {
    static const unsigned int TYPE = 9;
    uint32_t data[4];
    struct {
        uint32_t Rsvd1;
        uint32_t Rsvd2;
        uint32_t Rsvd3;

        uint32_t cycle_bit: 1;
        uint32_t          : 9;
        uint32_t  trb_type: 6;
        uint32_t slot_type: 5;
        uint32_t          : 11;
    } __attribute__((packed)) bits;
};

union TramsferEventTRB {
    static const unsigned int TYPE = 32;
    uint32_t data[4];
    struct {
        uint64_t trb_pointer;

        uint32_t trb_transfer_length: 24;
        uint32_t     completion_code: 8;

        uint32_t    cycle_bit: 1;
        uint32_t             : 1;
        uint32_t   event_data: 1;
        uint32_t             : 7;
        uint32_t     trb_type: 6;
        uint32_t endpointt_id: 5;
        uint32_t             : 3;
        uint32_t      slot_id: 8;
    } __attribute__((packed)) bits;
};

union CommandCompletionEventTRB {
    static const unsigned int TYPE = 33;
    uint32_t data[4];
    struct {
        uint64_t                    : 4;
        uint64_t command_trb_pointer: 60;

        uint32_t command_completion_pointer: 24;
        uint32_t            completion_code: 8;

        uint32_t cycle_bit: 1;
        uint32_t          : 9;
        uint32_t  trb_type: 6;
        uint32_t     vf_id: 8;
        uint32_t   slot_id: 8;
    } __attribute__((packed)) bits;
};

union PortStatusChangeEventTRB {
    static const unsigned int TYPE = 34;
    uint32_t data[4];
    struct {
        uint32_t        : 24;
        uint32_t port_id: 8;

        uint32_t Rsvd;

        uint32_t                : 24;
        uint32_t completion_code: 8;

        uint32_t cycle_bit: 1;
        uint32_t          : 9;
        uint32_t  trb_type: 6;
        uint32_t          : 16;
    } __attribute__((packed)) bits;
};