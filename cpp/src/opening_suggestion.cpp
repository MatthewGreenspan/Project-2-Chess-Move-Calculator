#include "opening_suggestion.hpp"

using namespace std;
// quick check for normal king san moves only
bool sanIsNonCastlingKingMove(const string& san) {
  if (san.empty()) return false;
  if (san.size() >= 5 && san.compare(0, 5, "O-O-O") == 0) return false;
  if (san.size() >= 3 && san.compare(0, 3, "O-O") == 0) return false;
  return san[0] == 'K';
}
