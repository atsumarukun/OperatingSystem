#include "graphics/logger.hpp"

#include <stdio.h>

void Logger::Debug(const char* format, ...) {
    va_list arg;
    int result;
    char s[1024];

    va_start(arg, format);
    result = vsprintf(s, format, arg);
    va_end(arg);

    console_.Push(s);
}

void Logger::Error(class Error error) {
    console_.Push("error: ", 0x0000ff);
    console_.Push(error.Message());
}