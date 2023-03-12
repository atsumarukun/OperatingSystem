#pragma once

#include "registry.hpp"
#include "context.hpp"
#include "ring.hpp"
#include "graphics/graphics.hpp"
#include "setupdata.hpp"
#include "endpoint.hpp"
#include "arraymap.hpp"

class ClassDriver;

class USBDevice {
    public:
        USBDevice(uint8_t slot_id, DoorbellRegister* doorbell_reg, FrameBufferWriter& frame_buffer_writer): slot_id_{slot_id}, doorbell_reg_{doorbell_reg}, frame_buffer_writer_{frame_buffer_writer} {}
        DeviceContext* DeviceContext() { return &device_context_; }
        InputContext* InputContext() { return &input_context_; }
        Ring* AllocateTransferRing(DeviceContextIndex index, int buffer_size, MemoryManager& memory_manager);
        void GetDescriptor(EndpointID endpoint_id, uint8_t desc_type, uint8_t desc_index);
        void OnTransferEventReceived(const TransferEventTRB* trb);
        bool IsInitialized();
        EndpointConfig* EndpointConfigs() { return ep_configs_; }
        int NumEndpointConfigs() { return num_ep_configs_; }
        uint8_t SlotID() { return slot_id_; }
        void OnEndpointsConfigured();
        void ControlOut(EndpointID endpoint_id, SetupData setupdata, ClassDriver* issuer);
        void InterruptIn(EndpointID endpoint_id, uint8_t* buffer, int len);

    private:
        FrameBufferWriter& frame_buffer_writer_;
        uint8_t slot_id_;
        DoorbellRegister* doorbell_reg_;
        alignas(64) struct DeviceContext device_context_;
        alignas(64) struct InputContext input_context_;
        Ring* transfer_rings_[31];
        uint8_t buffer_[256];
        ArrayMap<SetupData, ClassDriver*, 4> event_waiters_{};
        ArrayMap<const TRB*, const SetupStageTRB*, 16> setup_stage_map_{};
        uint8_t initialize_phase_ = 1;
        uint8_t num_configurations_;
        uint8_t config_index_;
        int num_ep_configs_;
        EndpointConfig ep_configs_[16];
        ClassDriver* classdrivers_[16];

        SetupStageTRB CreateSetupStageTRB(SetupData setupdata, int transfer_type);
        DataStageTRB CreateDataStageTRB(bool dir_in);
        void ControlIn(EndpointID endpoint_id, SetupData setupdata, ClassDriver* issuer);
        void SetConfiguration(EndpointID endpoint_id, uint8_t configuration_value);
        void Initialize(uint8_t* buffer);
        void Initialize(uint8_t* buffer, int len);
        void Initialize(uint8_t config_value);
        void OnControlCompleted(SetupData setupdata, uint8_t* buffer, int len);
        void OnInterruptCompleted(uint8_t endpoint_id, uint8_t* buffer, int len);
};