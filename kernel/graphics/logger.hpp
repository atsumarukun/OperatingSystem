#pragma once

#include "graphics/console.hpp"
#include "error/error.hpp"

class Logger  {
    public:
        Logger(Console& console): console_{console} {}
        void Debug(const char* format, ...);
        void Error(Error error);

    private:
        Console& console_;
};