#pragma once

#include <string>

using namespace std;

// used so weird king walks dont get picked early in openings
bool sanIsNonCastlingKingMove(const string& san);
