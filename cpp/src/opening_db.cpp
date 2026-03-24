#include "opening_db.hpp"
#include "pgn_parser.h"
#include <fstream>
#include <iostream>

ChessTrie g_trie;

void loadOpeningDatabase() {
  const char* pgnPath = "lichess_games.pgn";
  std::ifstream check(pgnPath);
  if (check.good()) {
    check.close();
    std::cout << "Loading opening database...\n";
    PGNParser parser(1200, 1600, 20);
    parser.parse(pgnPath, [&](const GameData& game) { g_trie.insertGame(game.moves); });
    g_trie.prune(5);
    std::cout << "Database ready.\n";
  } else {
    std::cout << "Opening database skipped (place " << pgnPath << " next to the app to enable).\n";
  }
}
