#pragma once
#include <string>

using namespace std;
// trims fen down to the part we use as a position key
inline string fenToKey(const string& fen) {
    size_t first  = fen.find(' ');
    if (first == string::npos) return fen;
    size_t second = fen.find(' ', first + 1);
    if (second == string::npos) return fen;
    return fen.substr(0, second);
}
