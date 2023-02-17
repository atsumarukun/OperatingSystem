#pragma once

#include "registry.hpp"

class Port {
    public:
        Port(uint8_t port_id, PortRegisterSet& port_reg_set): port_id_{port_id}, port_reg_set_{port_reg_set} {}
        bool IsConnected() const;
        bool IsPortResetChanged() const;
        bool IsEnabled() const;
        void ClearPortResetChange();
        void Reset();
        uint8_t ID() const { return port_id_; }
        uint8_t Speed() const { return (uint8_t) port_reg_set_.PORTSC.Read().bits.PS; }

    private:
        const uint8_t port_id_;
        PortRegisterSet& port_reg_set_;
};