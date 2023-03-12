#pragma once

#include <stdint.h>

#include "context.hpp"

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

union NormalTRB {
    static const unsigned int TYPE = 1;
    uint32_t data[4];
    struct {
      uint64_t data_buffer_pointer;

      uint32_t trb_transfer_length : 17;
      uint32_t td_size : 5;
      uint32_t interrupter_target : 10;

      uint32_t cycle_bit : 1;
      uint32_t evaluate_next_trb : 1;
      uint32_t interrupt_on_short_packet : 1;
      uint32_t no_snoop : 1;
      uint32_t chain_bit : 1;
      uint32_t interrupt_on_completion : 1;
      uint32_t immediate_data : 1;
      uint32_t : 2;
      uint32_t block_event_interrupt : 1;
      uint32_t trb_type : 6;
      uint32_t : 16;
    } __attribute__((packed)) bits;

    NormalTRB() {
      bits.trb_type = TYPE;
    }
  };

union SetupStageTRB {
    static const unsigned int TYPE = 2;
    static const unsigned int kNoDataStage = 0;
    static const unsigned int kOutDataStage = 2;
    static const unsigned int kInDataStage = 3;

    uint32_t data[4];
    struct {
        uint32_t request_type : 8;
        uint32_t request : 8;
        uint32_t value : 16;

        uint32_t index : 16;
        uint32_t length : 16;

        uint32_t trb_transfer_length : 17;
        uint32_t : 5;
        uint32_t interrupter_target : 10;

        uint32_t cycle_bit : 1;
        uint32_t : 4;
        uint32_t interrupt_on_completion : 1;
        uint32_t immediate_data : 1;
        uint32_t : 3;
        uint32_t trb_type : 6;
        uint32_t transfer_type : 2;
        uint32_t : 14;
    } __attribute__((packed)) bits;

    SetupStageTRB() {
        bits.trb_type = TYPE;
        bits.immediate_data = true;
        bits.trb_transfer_length = 8;
    }
};

union DataStageTRB {
    static const unsigned int TYPE = 3;
    uint32_t data[4];
    struct {
        uint64_t data_buffer_pointer;

        uint32_t trb_transfer_length : 17;
        uint32_t td_size : 5;
        uint32_t interrupter_target : 10;

        uint32_t cycle_bit : 1;
        uint32_t evaluate_next_trb : 1;
        uint32_t interrupt_on_short_packet : 1;
        uint32_t no_snoop : 1;
        uint32_t chain_bit : 1;
        uint32_t interrupt_on_completion : 1;
        uint32_t immediate_data : 1;
        uint32_t : 3;
        uint32_t trb_type : 6;
        uint32_t direction : 1;
        uint32_t : 15;
    } __attribute__((packed)) bits;

    DataStageTRB() {
        bits.trb_type = TYPE;
    }
};

union StatusStageTRB {
    static const unsigned int TYPE = 4;
    uint32_t data[4];
    struct {
        uint64_t : 64;

        uint32_t : 22;
        uint32_t interrupter_target : 10;

        uint32_t cycle_bit : 1;
        uint32_t evaluate_next_trb : 1;
        uint32_t : 2;
        uint32_t chain_bit : 1;
        uint32_t interrupt_on_completion : 1;
        uint32_t : 4;
        uint32_t trb_type : 6;
        uint32_t direction : 1;
        uint32_t : 15;
    } __attribute__((packed)) bits;

    StatusStageTRB() {
        bits.trb_type = TYPE;
    }
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
    EnableSlotCommandTRB () {
    bits.trb_type = TYPE;
   }
};

union AddressDeviceCommandTRB {
    static const unsigned int TYPE = 11;
    uint32_t data[4];
    struct {
        uint64_t                      : 4;
        uint64_t input_context_pointer: 60;

        uint32_t Rsvd;

        uint32_t                 cycle_bit: 1;
        uint32_t                          : 8;
        uint32_t block_set_address_request: 1;
        uint32_t                  trb_type: 6;
        uint32_t                          : 8;
        uint32_t                   slot_id: 8;
    } __attribute__((packed)) bits;
    AddressDeviceCommandTRB(const InputContext* input_context, uint8_t slot_id) {
        bits.trb_type = TYPE;
        bits.slot_id = slot_id;
        bits.input_context_pointer = (uint64_t) input_context >> 4;
    }
};

  union ConfigureEndpointCommandTRB {
    static const unsigned int TYPE = 12;
    uint32_t data[4];
    struct {
      uint64_t : 4;
      uint64_t input_context_pointer : 60;

      uint32_t : 32;

      uint32_t cycle_bit : 1;
      uint32_t : 8;
      uint32_t deconfigure : 1;
      uint32_t trb_type : 6;
      uint32_t : 8;
      uint32_t slot_id : 8;
    } __attribute__((packed)) bits;

    ConfigureEndpointCommandTRB(const InputContext* input_context, uint8_t slot_id) {
      bits.trb_type = TYPE;
      bits.slot_id = slot_id;
      bits.input_context_pointer = (uint64_t) input_context >> 4;
    }
  };

union TransferEventTRB {
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
        uint32_t  endpoint_id: 5;
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