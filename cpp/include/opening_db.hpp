#pragma once

#include "chess_opening_hash.h"
#include "chess_trie.h"

using namespace std;
// loads pgn data into the opening lookup structs
void loadOpeningDatabase();
string getOpeningPgnPath();
string lookupPositionDBMove(const string& fen);
// shared opening lookup data used by the app
extern ChessTrie g_trie;
extern OpeningHashMap g_openingHash;
