#pragma  once

#include "hid.hpp"

#include <functional>

class MouseDriver: public HIDDriver {
    public:
        MouseDriver(USBDevice* device, int index);

        void OnDataReceived() override;

        using ObserverType = void (int8_t displacement_x, int8_t displacement_y);
        void SubscribeMouseMove(std::function<ObserverType> observer);
        static std::function<ObserverType> default_observer;

    private:
        std::function<ObserverType> observers_[4];
        int num_observers_ = 0;

        void NotifyMouseMove(int8_t displacement_x, int8_t displacement_y);
};