#pragma once

#include "chess_opening_hash.h"
#include "chess_trie.h"

void loadOpeningDatabase();
std::string lookupPositionDBMove(const std::string& fen); 
extern ChessTrie g_trie;
extern OpeningHashMap g_openingHash;
