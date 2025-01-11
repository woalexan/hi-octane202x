//
// Created by martin on 1/11/25.
//

#ifndef LOGGING_H
#define LOGGING_H

#include <iostream>

const char* RED = "\033[0;31m";
const char* GREEN = "\033[0;32m";
const char* NORMAL = "\033[0m";

namespace logging {
    inline void Message(const char* level, const char* color, const char* message) {
        std::cout << color << level << message << NORMAL << std::endl;
    }

    bool Wrap(const char* message, bool func()) {
        return func();
    }

    inline void Info(const char* message) {
        Message("INFO: ", GREEN, message);
    }

    inline void Error(const char* message) {
        Message("ERROR: ", RED, message);
    }
}

#endif //LOGGING_H
