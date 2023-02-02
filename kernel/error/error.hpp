#pragma once

#include <stdio.h>
#include <array>

using namespace std;

#define MakeError(error_code) Error(error_code, __FILE__, __LINE__)

class Error {
    public:
        enum ErrorCode {
            Success,
            MemoryFrameShortage,
            PciBarIndexOutOfRange,
        };
        Error(ErrorCode error_code, const char* file, int line): error_code_{error_code} {sprintf(error_message_, "%s:%d: %s", file, line, error_names_[error_code]);}
        char* Message() {return &error_message_[0];}
        operator bool() const {
            return this->error_code_ != Success;
        }

    private:
        enum ErrorCode error_code_;
        char error_message_[1024];
        static constexpr array error_names_ {
            "Success",
            "MemoryFrameShortage",
            "PciBarIndexOutOfRange",
        };
};

template <class T>
struct WithError{
    T value;
    Error error;
};