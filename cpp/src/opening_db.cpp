#include "opening_db.hpp"
#include "pgn_parser.h"
#include "position_db.hpp"
#include "fen_parser.hpp"
#include "chess.hpp"
#include <fstream>
#include <iostream>
#include <string>

using namespace chess;

ChessTrie g_trie;
OpeningHashMap g_openingHash;

static std::string findPgnPath() {
  const char* tries[] = {"lichess_games.pgn", "cpp/lichess_games.pgn", "../cpp/lichess_games.pgn"};
  for (const char* p : tries) {
    std::ifstream t(p);
    if (t.good()) return p;
  }
  return "";
}

std::string getOpeningPgnPath() { return findPgnPath(); }

static constexpr int MAX_PLY = 20;

void loadOpeningDatabase() {
  std::string pgnPath = findPgnPath();
  if (pgnPath.empty()) {
    std::cout << "Opening database skipped (place lichess_games.pgn next to the app or under cpp/).\n";
    return;
  }
  std::cout << "Loading opening database from " << pgnPath << "...\n";

  PGNParser parser(1000, 2500, 40);
  parser.parse(pgnPath, [&](const GameData& game) {
    g_trie.insertGame(game.moves);
    g_openingHash.insertGame(game.moves);

    Board board(constants::STARTPOS);
    for (int i = 0; i < (int)game.moves.size() && i < MAX_PLY; ++i) {
      const std::string& san = game.moves[i];
      std::string key = fenToKey(board.getFen());

      Move move = Move::NO_MOVE;
      try {
        move = uci::parseSan(board, san);
      } catch (...) {
        break;
      }
      if (move == Move::NO_MOVE) break;

      g_positionDB.record(key, san);
      board.makeMove(move);
    }
  });

  g_trie.prune(1);
  g_openingHash.prune(1);
  g_positionDB.prune(1);

  g_positionDB.printStats();
  std::cout << "Opening database ready.\n";
}

std::string lookupPositionDBMove(const std::string& fen) { return g_positionDB.bestMove(fenToKey(fen)); }
