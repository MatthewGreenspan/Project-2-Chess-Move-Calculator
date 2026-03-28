#pragma once

#include <string>

using namespace std;
/** True for king moves like Ke2, Kxf7 — not castling (O-O / O-O-O). */
bool sanIsNonCastlingKingMove(const string& san);
