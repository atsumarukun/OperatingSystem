/*
@file port.cpp

xhcのポートファイル.
*/

#include "port.hpp"

bool Port::IsConnected() const {
    return port_reg_set_.PORTSC.Read().bits.CCS;
}

bool Port::IsPortResetChanged() const {
return port_reg_set_.PORTSC.Read().bits.PRC;
}

bool Port::IsEnabled() const {
    return port_reg_set_.PORTSC.Read().bits.PED;
}

void Port::ClearPortResetChange() {
    PORTSCMap portsc = port_reg_set_.PORTSC.Read();
    portsc.data[0] &= 0x0e01c3e0u;
    portsc.bits.PRC = 1;
    port_reg_set_.PORTSC.Write(portsc);
}

void Port::Reset() {
    auto portsc = port_reg_set_.PORTSC.Read();
    portsc.data[0] &= 0x0e00c3e0u;
    portsc.data[0] |= 0x00020010u;
    port_reg_set_.PORTSC.Write(portsc);
    while (port_reg_set_.PORTSC.Read().bits.PR);
}
