#pragma once

#include "device.hpp"
#include "setupdata.hpp"

class ClassDriver {
    public:
        ClassDriver(USBDevice* device);
        virtual ~ClassDriver();

        virtual void SetEndpoint(EndpointConfig& endpoint_config) = 0;
        virtual void OnEndpointsConfigured() = 0;
        virtual void OnControlCompleted() = 0;
        virtual void OnInterruptCompleted(EndpointID ep_id, uint8_t* buffer, int len) = 0;
        USBDevice* Device() const { return device_; }

    private:
        USBDevice* device_;
};