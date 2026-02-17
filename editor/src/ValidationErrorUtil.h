#pragma once

#include <string>

inline bool failWith(std::string* error, const char* code) {
    if (!error) return false;
    *error = code;
    return false;
}
