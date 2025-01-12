//
// Created by martin on 1/11/25.
//

#ifndef LOGGING_H
#define LOGGING_H

#include <iostream>

const char* RED = "\033[0;31m";
const char* GREEN = "\033[0;32m";
const char* CYAN = "\033[0;36m";
const char* GREY = "\033[0;90m";
const char* NORMAL = "\033[0m";

namespace logging {
    inline void Message(const char* level, const char* color, const char* message) {
        std::cout << color << level << message << NORMAL << std::endl;
    }
    inline void Message(const char* level, const char* color, const std::string &message) {
        std::cout << color << level << message << NORMAL << std::endl;
    }

    template<typename T>
    void Error(T message) {
        Message("ERROR: ", RED, message);
    }

    template<typename T>
    void Info(T message) {
        Message("INFO: ", GREEN, message);
    }

    template<typename T>
    void Detail(T message) {
        Message("DETAIL: ", CYAN, message);
    }

    template<typename T>
    void Debug(T message) {
        Message("DEBUG: ", GREY, message);
    }
}

#endif //LOGGING_H
