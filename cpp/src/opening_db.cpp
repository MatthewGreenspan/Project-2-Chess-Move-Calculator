#include "opening_db.hpp"
#include "pgn_parser.h"
#include <fstream>
#include <iostream>
#include <string>

ChessTrie g_trie;

static std::string findPgnPath() {
  const char* tries[] = {"lichess_games.pgn", "cpp/lichess_games.pgn", "../cpp/lichess_games.pgn"};
  for (const char* p : tries) {
    std::ifstream t(p);
    if (t.good()) return p;
  }
  return "";
}

void loadOpeningDatabase() {
  std::string pgnPath = findPgnPath();
  if (!pgnPath.empty()) {
    std::cout << "Loading opening database from " << pgnPath << "...\n";
    PGNParser parser(1800, 2500, 40);
    parser.parse(pgnPath, [&](const GameData& game) { g_trie.insertGame(game.moves); });
    g_trie.prune(5);
    std::cout << "Database ready.\n";
  } else {
    std::cout << "Opening database skipped (place lichess_games.pgn next to the app or under cpp/).\n";
  }
}
