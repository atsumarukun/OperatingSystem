#include "hid.hpp"
#include "graphics/logger.hpp"

extern Logger* logger;

HIDDriver::HIDDriver(USBDevice* device, int index, int size): ClassDriver{device}, index_{index}, size_{size} {}

void HIDDriver::SetEndpoint(EndpointConfig& endpoint_config) {
    if (endpoint_config.ep_type == EndpointType::kInterrupt && endpoint_config.ep_id.IsIn()) {
        endpoint_interrupt_in_ = endpoint_config.ep_id;
    } else if (endpoint_config.ep_type == EndpointType::kInterrupt && !endpoint_config.ep_id.IsIn()) {
        endpoint_interrupt_out_ = endpoint_config.ep_id;
    }
}

void HIDDriver::OnEndpointsConfigured() {
    logger->Debug("classdriver:%p Configured endpoints.\n", this);
    SetupData setupdata{};
    setupdata.request_type.bits.direction = request_type::kOut;
    setupdata.request_type.bits.type = request_type::kClass;
    setupdata.request_type.bits.recipient = request_type::kInterface;
    setupdata.request = request::kSetProtocol;
    setupdata.value = 0;
    setupdata.index = index_;
    setupdata.length = 0;

    initialize_phase_ = 1;
    Device()->ControlOut(kDefaultControlPipeID, setupdata, this);
}

void HIDDriver::OnControlCompleted() {
    if (initialize_phase_ == 1) {
        logger->Debug("classdriver:%p Completed control.\n", this);
        initialize_phase_ = 2;
        Device()->InterruptIn(endpoint_interrupt_in_, buffer_, size_);
    }
}

void HIDDriver::OnInterruptCompleted(EndpointID ep_id, uint8_t* buffer, int len) {
    if (ep_id.IsIn()) {
        OnDataReceived();
        Device()->InterruptIn(endpoint_interrupt_in_, buffer_, size_);
    }
}