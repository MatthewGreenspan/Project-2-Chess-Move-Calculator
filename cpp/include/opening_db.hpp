#pragma once

#include "chess_opening_hash.h"
#include "chess_trie.h"

using namespace std;
void loadOpeningDatabase();
/** Path to lichess_games.pgn if found (same search as loader), else empty. */
string getOpeningPgnPath();
string lookupPositionDBMove(const string& fen);
extern ChessTrie g_trie;
extern OpeningHashMap g_openingHash;
