#pragma once

#include "chess_opening_hash.h"
#include "chess_trie.h"

void loadOpeningDatabase();
/** Path to lichess_games.pgn if found (same search as loader), else empty. */
std::string getOpeningPgnPath();
std::string lookupPositionDBMove(const std::string& fen);
extern ChessTrie g_trie;
extern OpeningHashMap g_openingHash;
