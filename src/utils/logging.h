//
// Created by martin on 1/11/25.
//

#ifndef LOGGING_H
#define LOGGING_H

#include <iostream>

namespace logging {
    inline const char* RED = "\033[0;31m";
    inline const char* GREEN = "\033[0;32m";
    inline const char* YELLOW = "\033[0;33m";
    inline const char* CYAN = "\033[0;36m";
    inline const char* GREY = "\033[0;90m";
    inline const char* NORMAL = "\033[0m";

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
    void Warning(T message) {
        Message("WARNING: ", YELLOW, message);
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
