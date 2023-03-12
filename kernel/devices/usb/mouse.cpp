#include "mouse.hpp"

MouseDriver::MouseDriver(USBDevice* device, int index): HIDDriver{device, index, 3} {}

void MouseDriver::OnDataReceived() {
    int8_t displacement_x = Buffer()[1];
    int8_t displacement_y = Buffer()[2];
    NotifyMouseMove(displacement_x, displacement_y);
}

void MouseDriver::SubscribeMouseMove(std::function<void (int8_t displacement_x, int8_t displacement_y)> observer) {
  observers_[num_observers_++] = observer;
}

std::function<MouseDriver::ObserverType> MouseDriver::default_observer;

void MouseDriver::NotifyMouseMove(int8_t displacement_x, int8_t displacement_y) {
  for (int i = 0; i < num_observers_; i++) {
    observers_[i](displacement_x, displacement_y);
  }
}