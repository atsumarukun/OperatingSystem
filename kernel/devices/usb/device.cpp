#include "device.hpp"
#include "descriptor.hpp"
#include "classdriver.hpp"
#include "mouse.hpp"
#include "keyboard.hpp"
#include "graphics/logger.hpp"

extern Logger* logger;

namespace {
    class ConfigurationDescriptorReader {
        public:
            ConfigurationDescriptorReader(uint8_t* buffer, int len): buffer_{buffer}, len_{len}, pointer_{buffer} {}
            uint8_t* Next() {
                pointer_ += pointer_[0];
                if (pointer_ < buffer_ + len_) {
                    return pointer_;
                }
                return nullptr;
            }

            template <class T>
            T* Next() {
                while (uint8_t* pointer = Next()) {
                    if (T* descriptor = DescriptorTypeCast<T>(pointer)) {
                        return descriptor;
                    }
                }
                return nullptr;
            }

        private:
            uint8_t* buffer_;
            int len_;
            uint8_t* pointer_;
    };

    ClassDriver* NewClassDriver(USBDevice* device, InterfaceDescriptor* interface_descriptor) {
        if (interface_descriptor->interface_class == 3 && interface_descriptor->interface_sub_class == 1) {
            if (interface_descriptor->interface_protocol == 1) {
                KeyboardDriver* keyboard_driver = new KeyboardDriver{device, interface_descriptor->interface_number};
                if (KeyboardDriver::default_observer) {
                    keyboard_driver->SubscribeKeyPush(KeyboardDriver::default_observer);
                }
                return keyboard_driver;
            } else if (interface_descriptor->interface_protocol == 2) {
                MouseDriver* mouse_driver = new MouseDriver{device, interface_descriptor->interface_number};
                if (MouseDriver::default_observer) {
                    mouse_driver->SubscribeMouseMove(MouseDriver::default_observer);
                }
                return mouse_driver;
            }
        }
        return nullptr;
    }

    EndpointConfig CreateEndpointConfig(EndpointDescriptor& endpoint_descriptor) {
        EndpointConfig endpoint_config;
        endpoint_config.ep_id = EndpointID{
            endpoint_descriptor.endpoint_address.bits.number,
            endpoint_descriptor.endpoint_address.bits.dir_in == 1
        };
        endpoint_config.ep_type = (EndpointType) endpoint_descriptor.attributes.bits.transfer_type;
        endpoint_config.max_packet_size = endpoint_descriptor.max_packet_size;
        endpoint_config.interval = endpoint_descriptor.interval;
        return endpoint_config;
    }
}

Ring* USBDevice::AllocateTransferRing(DeviceContextIndex index, int buffer_size, MemoryManager& memory_manager) {
    int i = index.value - 1;
    Ring* tr = (Ring*) memory_manager.Allocate(1).value;
    logger->Debug("device:%p: Allocating transfer ring(%p).\n", this, tr);
    transfer_rings_[i] = tr;
    tr->Initialize(buffer_size, memory_manager);
    return tr;
}

void USBDevice::GetDescriptor(EndpointID endpoint_id, uint8_t desc_type, uint8_t desc_index) {
    SetupData setupdata{};
    setupdata.request_type.bits.direction = request_type::kIn;
    setupdata.request_type.bits.type = request_type::kStandard;
    setupdata.request_type.bits.recipient = request_type::kDevice;
    setupdata.request = request::kGetDescriptor;
    setupdata.value = ((uint16_t) desc_type << 8) | desc_index;
    setupdata.index = 0;
    setupdata.length = sizeof(buffer_);
    ControlIn(endpoint_id, setupdata, nullptr);
}

void USBDevice::OnTransferEventReceived(const TransferEventTRB* trb) {
    const uint32_t residual_length = trb->bits.trb_transfer_length;
    if (trb->bits.completion_code != 1 && trb->bits.completion_code != 13) return;

    TRB* issuer_trb = (TRB*) trb->bits.trb_pointer;
    if (NormalTRB* event_trb = TrbTypeCast<NormalTRB>(issuer_trb)) {
        const int transfer_length = event_trb->bits.trb_transfer_length - residual_length;
        OnInterruptCompleted(trb->bits.endpoint_id, (uint8_t*) event_trb->bits.data_buffer_pointer, transfer_length);
    }

    auto opt_setup_stage_trb = setup_stage_map_.Get(issuer_trb);
    if (!opt_setup_stage_trb) return;
    setup_stage_map_.Delete(issuer_trb);

    auto setup_stage_trb = opt_setup_stage_trb.value();
    SetupData setupdata{};
    setupdata.request_type.data = setup_stage_trb->bits.request_type;
    setupdata.request = setup_stage_trb->bits.request;
    setupdata.value = setup_stage_trb->bits.value;
    setupdata.index = setup_stage_trb->bits.index;
    setupdata.length = setup_stage_trb->bits.length;

    uint8_t* buffer = nullptr;
    int transfer_length = 0;
    if (DataStageTRB* event_trb = TrbTypeCast<DataStageTRB>(issuer_trb)) {
        buffer = (uint8_t*) event_trb->bits.data_buffer_pointer;
        transfer_length = event_trb->bits.trb_transfer_length - residual_length;
    }
    OnControlCompleted(setupdata, buffer, transfer_length);
}

bool USBDevice::IsInitialized() {
    if (initialize_phase_ == 4) {
        initialize_phase_ = 5;
        return true;
    }
    return false;
}

void USBDevice::OnEndpointsConfigured() {
    for (ClassDriver* classdriver: classdrivers_) {
        if (classdriver) {
            classdriver->OnEndpointsConfigured();
        }
    }
}

void USBDevice::ControlOut(EndpointID endpoint_id, SetupData setupdata, ClassDriver* issuer) {
    if (issuer) {
        event_waiters_.Put(setupdata, issuer);
    }

    const DeviceContextIndex dci{endpoint_id};
    Ring* tr = transfer_rings_[dci.value - 1];
    StatusStageTRB status = StatusStageTRB{};
    status.bits.direction = true;
    status.bits.interrupt_on_completion = true;

    SetupStageTRB* setup_trb = TrbTypeCast<SetupStageTRB>(tr->Push(CreateSetupStageTRB(setupdata, SetupStageTRB::kNoDataStage).data));
    TRB* status_trb = tr->Push(status.data);
    setup_stage_map_.Put(status_trb, setup_trb);

    doorbell_reg_->Ring(dci.value);
}

void USBDevice::InterruptIn(EndpointID endpoint_id, uint8_t* buffer, int len) {
    DeviceContextIndex dci{endpoint_id};
    Ring* tr = transfer_rings_[dci.value - 1];

    NormalTRB normal{};
    normal.bits.data_buffer_pointer = (uint64_t) buffer;
    normal.bits.trb_transfer_length = len;
    normal.bits.interrupt_on_short_packet = true;
    normal.bits.interrupt_on_completion = true;

    tr->Push(normal.data);
    doorbell_reg_->Ring(dci.value);
}

SetupStageTRB USBDevice::CreateSetupStageTRB(SetupData setupdata, int transfer_type) {
    SetupStageTRB setup{};
    setup.bits.request_type = setupdata.request_type.data;
    setup.bits.request = setupdata.request;
    setup.bits.value = setupdata.value;
    setup.bits.index = setupdata.index;
    setup.bits.length = setupdata.length;
    setup.bits.transfer_type = transfer_type;
    return setup;
}

DataStageTRB USBDevice::CreateDataStageTRB(bool dir_in) {
    DataStageTRB data{};
    data.bits.data_buffer_pointer = (uint64_t) buffer_;
    data.bits.trb_transfer_length = sizeof(buffer_);
    data.bits.td_size = 0;
    data.bits.direction = dir_in;
    return data;
}

void USBDevice::ControlIn(EndpointID endpoint_id, SetupData setupdata, ClassDriver* issuer) {
    if (issuer) {
      event_waiters_.Put(setupdata, issuer);
    }

    const DeviceContextIndex dci{endpoint_id};
    Ring* tr = transfer_rings_[dci.value - 1];
    StatusStageTRB status = StatusStageTRB{};

    SetupStageTRB* setup_trb = TrbTypeCast<SetupStageTRB>(tr->Push(CreateSetupStageTRB(setupdata, SetupStageTRB::kInDataStage).data));
    DataStageTRB data = CreateDataStageTRB(true);
    data.bits.interrupt_on_completion = true;
    TRB* data_trb = tr->Push(data.data);
    tr->Push(status.data);
    setup_stage_map_.Put(data_trb, setup_trb);

    doorbell_reg_->Ring(dci.value);
}

void USBDevice::SetConfiguration(EndpointID endpoint_id, uint8_t configuration_value) {
    SetupData setupdata{};
    setupdata.request_type.bits.direction = request_type::kOut;
    setupdata.request_type.bits.type = request_type::kStandard;
    setupdata.request_type.bits.recipient = request_type::kDevice;
    setupdata.request = request::kSetConfiguration;
    setupdata.value = configuration_value;
    setupdata.index = 0;
    setupdata.length = 0;
    ControlOut(endpoint_id, setupdata, nullptr);
}

void USBDevice::Initialize(uint8_t* buffer) {
    logger->Debug("device:%p: Initializing phase 1.\n", this);
    DeviceDescriptor* device_descriptor = DescriptorTypeCast<DeviceDescriptor>(buffer);
    num_configurations_ = device_descriptor->num_configurations;
    config_index_ = 0;
    initialize_phase_ = 2;
    GetDescriptor(kDefaultControlPipeID, ConfigurationDescriptor::TYPE, config_index_);
}

void USBDevice::Initialize(uint8_t* buffer, int len) {
    logger->Debug("device:%p: Initializing phase 2.\n", this);
    ConfigurationDescriptor* configuration_descriptor = DescriptorTypeCast<ConfigurationDescriptor>(buffer);
    ConfigurationDescriptorReader configuration_reader = ConfigurationDescriptorReader{buffer, len};

    ClassDriver* classdriver = nullptr;
    while (InterfaceDescriptor* interface_descriptor = configuration_reader.Next<InterfaceDescriptor>()) {
        classdriver = NewClassDriver(this, interface_descriptor);
        if (!classdriver) continue;

        num_ep_configs_ = 0;

        while (num_ep_configs_ < interface_descriptor->num_endpoints) {
            uint8_t* descriptor = configuration_reader.Next();
            if (EndpointDescriptor* endpoint_descriptor = DescriptorTypeCast<EndpointDescriptor>(descriptor)) {
                EndpointConfig endpoint_config = CreateEndpointConfig(*endpoint_descriptor);
                ep_configs_[num_ep_configs_++] = endpoint_config;
                classdrivers_[endpoint_config.ep_id.Number()] = classdriver;
            }
        }
        break;
    }

    initialize_phase_ = 3;
    SetConfiguration(kDefaultControlPipeID, configuration_descriptor->configuration_value);
}

void USBDevice::Initialize(uint8_t config_value) {
    logger->Debug("device:%p: Initializing phase 3.\n", this);
    for (int i = 0; i < num_ep_configs_; i++) {
        classdrivers_[ep_configs_[i].ep_id.Number()]->SetEndpoint(ep_configs_[i]);
    }

    initialize_phase_ = 4;
}

void USBDevice::OnControlCompleted(SetupData setupdata, uint8_t* buffer, int len) {
    switch (initialize_phase_) {
        case 1:
            if (setupdata.request == request::kGetDescriptor && DescriptorTypeCast<DeviceDescriptor>(buffer)) {
                Initialize(buffer);
            }
            break;
        case 2:
            if (setupdata.request == request::kGetDescriptor && DescriptorTypeCast<ConfigurationDescriptor>(buffer)) {
                Initialize(buffer, len);
            }
            break;
        case 3:
            if (setupdata.request == request::kSetConfiguration) {
                Initialize(setupdata.value);
            }
            break;
        case 5:
            ClassDriver* classdriver = event_waiters_.Get(setupdata).value();
            classdriver->OnControlCompleted();
            break;
    }
}

void USBDevice::OnInterruptCompleted(uint8_t endpoint_id, uint8_t* buffer, int len) {
    EndpointID ep_id{endpoint_id};
    ClassDriver* classdriver = classdrivers_[ep_id.Number()];
    classdriver->OnInterruptCompleted(ep_id, buffer, len);
}