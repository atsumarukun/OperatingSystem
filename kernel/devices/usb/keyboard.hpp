#pragma  once

#include "hid.hpp"

#include <functional>

class KeyboardDriver: public HIDDriver {
    public:
        KeyboardDriver(USBDevice* device, int index);

        void OnDataReceived() override;

        using ObserverType = void (uint8_t keycode);
        void SubscribeKeyPush(std::function<ObserverType> observer);
        static std::function<ObserverType> default_observer;

    private:
        std::function<ObserverType> observers_[4];
        int num_observers_ = 0;
};