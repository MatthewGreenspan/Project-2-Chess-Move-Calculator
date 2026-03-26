#pragma once
#include <string>

inline std::string fenToKey(const std::string& fen) {
    size_t first  = fen.find(' ');
    if (first == std::string::npos) return fen;
    size_t second = fen.find(' ', first + 1);
    if (second == std::string::npos) return fen;
    return fen.substr(0, second);
}