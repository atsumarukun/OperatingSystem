#pragma once

#include <stdint.h>

template <typename T>
class MMIO {
    public:
        T Read() const {
            T tmp;
            for (int i = 0; i < size_; i++) {
                tmp.data[i] = value_.data[i];
            }
            return tmp;
        }

        void Write(const T& value) {
            for (int i = 0; i < size_; i++) {
                value_.data[i] = value.data[i];
            }
        }

    private:
        volatile T value_;
        static const int size_ = sizeof(T) / sizeof(value_.data[0]);
};

template <typename T>
class Array {
    public:
        Array(uintptr_t base_address, int size): array_{(T*) base_address}, size_{size} {}
        T& operator [](int index) { return array_[index]; }

    private:
        T* array_;
        int size_;
};

union HCSPARAMS1Map {
    uint32_t data[1];
    struct {
        uint32_t MaxSlots: 8;
        uint32_t MaxIntrs: 11;
        uint32_t         : 5;
        uint32_t MaxPorts: 8;
    } __attribute__((packed)) bits;
} __attribute__((packed));

union HCSPARAMS2Map {
    uint32_t data[1];
    struct {
        uint32_t                 IST: 4;
        uint32_t             ERSTMax: 4;
        uint32_t                    : 13;
        uint32_t MaxScratchpadBufsHi: 5;
        uint32_t                 SPR: 1;
        uint32_t MaxScratchpadBufsLo: 5;
    } __attribute__((packed)) bits;
} __attribute__((packed));

union HCSPARAMS3Map {
    uint32_t data[1];
    struct {
        uint32_t U1DeviceExitLatency: 8;
        uint32_t                    : 8;
        uint32_t U2DeviceExitLatency: 16;
    } __attribute__((packed)) bits;
} __attribute__((packed));

union HCCPARAMS1Map {
    uint32_t data[1];
    struct {
        uint32_t       AC64: 1;
        uint32_t        BNC: 1;
        uint32_t        CSZ: 1;
        uint32_t        PPC: 1;
        uint32_t       PIND: 1;
        uint32_t       LHRC: 1;
        uint32_t        LTC: 1;
        uint32_t        NSS: 1;
        uint32_t        PAE: 1;
        uint32_t        SPC: 1;
        uint32_t        SEC: 1;
        uint32_t        CFC: 1;
        uint32_t MaxPSASize: 4;
        uint32_t       xECP: 16;
    } __attribute__((packed)) bits;
} __attribute__((packed));

union HCCPARAMS2Map {
    uint32_t data[1];
    struct {
        uint32_t     U3C: 1;
        uint32_t     CMC: 1;
        uint32_t     FSC: 1;
        uint32_t     CTC: 1;
        uint32_t     LEC: 1;
        uint32_t     CIC: 1;
        uint32_t     ETC: 1;
        uint32_t ETC_TSC: 1;
        uint32_t     GSC: 1;
        uint32_t     VTD: 1;
        uint32_t        : 22;
    } __attribute__((packed)) bits;
} __attribute__((packed));

struct CapabilityRegisters {
    uint8_t CAPLENGTH;
    uint8_t Rsvd;
    uint16_t HCIVERSION;
    MMIO<HCSPARAMS1Map> HCSPARAMS1;
    MMIO<HCSPARAMS2Map> HCSPARAMS2;
    MMIO<HCSPARAMS3Map> HCSPARAMS3;
    MMIO<HCCPARAMS1Map> HCCPARAMS1;
    uint32_t DBOFF;
    uint32_t RTSOFF;
    MMIO<HCCPARAMS2Map> HCCPARAMS2;

} __attribute__((packed));

union USBCMDMap {
    uint32_t data[1];
    struct {
        uint32_t     RS: 1;
        uint32_t  HCRST: 1;
        uint32_t   INTE: 1;
        uint32_t   HSEE: 1;
        uint32_t       : 3;
        uint32_t LHCRST: 1;
        uint32_t    CSS: 1;
        uint32_t    CRS: 1;
        uint32_t    EWE: 1;
        uint32_t   EU3S: 1;
        uint32_t       : 1;
        uint32_t    CME: 1;
        uint32_t    ETE: 1;
        uint32_t TSC_EN: 1;
        uint32_t  VTIOE: 1;
        uint32_t       : 15;
    } __attribute__((packed)) bits;
} __attribute__((packed));

union USBSTSMap {
    uint32_t  data[1];
    struct {
        uint32_t  HCH: 1;
        uint32_t     : 1;
        uint32_t  HSE: 1;
        uint32_t EINT: 1;
        uint32_t  PCD: 1;
        uint32_t     : 3;
        uint32_t  SSS: 1;
        uint32_t  RSS: 1;
        uint32_t  SRE: 1;
        uint32_t  CNR: 1;
        uint32_t  HCE: 1;
        uint32_t     : 19;
    } __attribute__((packed)) bits;
} __attribute__((packed));

union CRCRMap {
    uint64_t data[1];
    struct {
        uint64_t RCS: 1;
        uint64_t  CS: 1;
        uint64_t  CA: 1;
        uint64_t CRR: 1;
        uint64_t    : 2;
        uint64_t CRP: 58;
    } __attribute__((packed)) bits;
} __attribute__((packed));

union DCBAAPMap {
    uint64_t data[1];
    struct {
        uint64_t       : 6;
        uint64_t DCBAAP: 26;
    } __attribute__((packed)) bits;
} __attribute__((packed));

union CONFIGMap {
    uint32_t data[1];
    struct {
        uint32_t MaxSlotsEn: 8;
        uint32_t        U3E: 1;
        uint32_t        CIE: 1;
        uint32_t           : 22;
    } __attribute__((packed)) bits;
} __attribute__((packed));

struct OperationalRegisters {
    MMIO<USBCMDMap> USBCMD;
    MMIO<USBSTSMap> USBSTS;
    uint32_t PAGESIZE;
    uint64_t Rsvd1;
    uint32_t DNCTRL;
    MMIO<CRCRMap> CRCR;
    uint64_t Rsvd2[2];
    MMIO<DCBAAPMap> DCBAAP;
    MMIO<CONFIGMap> CONFIG;
} __attribute__((packed));

union ERSTBAMap {
    uint64_t data[1];
    struct {
        uint64_t       : 6;
        uint64_t ERSTBA: 58;
    } __attribute__((packed)) bits;
} __attribute__((packed));

union ERDPMap {
    uint64_t data[1];
    struct {
        uint64_t DESI: 3;
        uint64_t  EHB: 1;
        uint64_t ERDP: 60;
    } __attribute__((packed)) bits;
} __attribute__((packed));

struct InterrupterRegisterSet {
    uint32_t IMAN;
    uint32_t IMOD;
    uint32_t ERSTSZ;
    uint32_t Rsvd;
    MMIO<ERSTBAMap> ERSTBA;
    MMIO<ERDPMap> ERDP;
} __attribute__((packed));

union PORTSCMap {
    uint32_t data[1];
    struct {
        uint32_t CCS: 1;
        uint32_t PED: 1;
        uint32_t    : 1;
        uint32_t OCA: 1;
        uint32_t  PR: 1;
        uint32_t PLS: 4;
        uint32_t  PP: 1;
        uint32_t  PS: 4;
        uint32_t PIC: 2;
        uint32_t LWS: 1;
        uint32_t CSC: 1;
        uint32_t PEC: 1;
        uint32_t WRC: 1;
        uint32_t OCC: 1;
        uint32_t PRC: 1;
        uint32_t PLC: 1;
        uint32_t CEC: 1;
        uint32_t CAS: 1;
        uint32_t WCE: 1;
        uint32_t WDE: 1;
        uint32_t WOE: 1;
        uint32_t    : 2;
        uint32_t  DR: 1;
        uint32_t WPR: 1;
    } __attribute__((packed)) bits;
} __attribute__((packed));

union PORTPMSCMap {
    uint32_t data[1];
    struct {
        uint32_t U1Timeout: 8;
        uint32_t U2Timeout: 8;
        uint32_t       FLA: 1;
        uint32_t          : 15;
    } __attribute__((packed)) bits;
} __attribute__((packed));

union PORTLIMap {
    uint32_t data[1];
    struct {
        uint32_t LEC: 16;
        uint32_t RLC: 4;
        uint32_t TLC: 4;
        uint32_t    : 8;
    } __attribute__((packed)) bits;
} __attribute__((packed));

union PORTHLPMCMap {
    uint32_t data[1];
    struct {
        uint32_t     HIRDM: 2;
        uint32_t L1Timeout: 8;
        uint32_t     BESLD: 4;
        uint32_t          : 18;
    } __attribute__((packed)) bits;
} __attribute__((packed));

struct PortRegisterSet {
    MMIO<PORTSCMap> PORTSC;
    MMIO<PORTPMSCMap> PORTPMSC;
    MMIO<PORTLIMap> PORTLI;
    MMIO<PORTHLPMCMap> PORTHLPMC;
} __attribute__((packed));