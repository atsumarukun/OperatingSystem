#pragma once

#include "classdriver.hpp"

class HIDDriver: public ClassDriver {
    public:
        HIDDriver(USBDevice* device, int index, int size);
        void SetEndpoint(EndpointConfig& endpoint_config) override;
        void OnEndpointsConfigured() override;
        void OnControlCompleted() override;
        void OnInterruptCompleted(EndpointID ep_id, uint8_t* buffer, int len) override;
        virtual void OnDataReceived() = 0;
        uint8_t* Buffer() { return buffer_; }

    private:
        int index_;
        int size_;
        EndpointID endpoint_interrupt_in_;
        EndpointID endpoint_interrupt_out_;
        int initialize_phase_ = 0;
        uint8_t buffer_[1024];
};