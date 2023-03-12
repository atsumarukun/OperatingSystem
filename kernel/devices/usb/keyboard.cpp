#include "keyboard.hpp"

KeyboardDriver::KeyboardDriver(USBDevice* device, int index): HIDDriver{device, index, 8} {}

void KeyboardDriver::OnDataReceived() {
    return;
}

void KeyboardDriver::SubscribeKeyPush(std::function<void (uint8_t keycode)> observer) {
    observers_[num_observers_++] = observer;
}

std::function<KeyboardDriver::ObserverType> KeyboardDriver::default_observer;