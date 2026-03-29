#include "opening_db.hpp"
#include "pgn_parser.h"
#include "position_db.hpp"
#include "fen_parser.hpp"
#include "chess.hpp"
#include <fstream>
#include <iostream>
#include <string>

using namespace std;
using namespace chess;

// shared opening structures used across the app
ChessTrie g_trie;
OpeningHashMap g_openingHash;

// checks the common spots for the pgn file
static string findPgnPath() {
  const char* tries[] = {"lichess_games.pgn", "cpp/lichess_games.pgn", "../cpp/lichess_games.pgn"};
  for (const char* p : tries) {
    ifstream t(p);
    if (t.good()) return p;
  }
  return "";
}

string getOpeningPgnPath() { return findPgnPath(); }

static constexpr int MAX_PLY = 20;

// loads opening data into trie/hash/position db
void loadOpeningDatabase() {
  string pgnPath = findPgnPath();
  if (pgnPath.empty()) {
    cout << "Opening database skipped (place lichess_games.pgn next to the app or under cpp/).\n";
    return;
  }
  cout << "Loading opening database from " << pgnPath << "...\n";

  PGNParser parser(1000, 2500, 40);
  parser.parse(pgnPath, [&](const GameData& game) {
    g_trie.insertGame(game.moves);
    g_openingHash.insertGame(game.moves);

    Board board(constants::STARTPOS);
    for (int i = 0; i < (int)game.moves.size() && i < MAX_PLY; ++i) {
      const string& san = game.moves[i];
      string key = fenToKey(board.getFen());

      Move move = Move::NO_MOVE;
      try {
        move = uci::parseSan(board, san);
      } catch (...) {
        // stop this game if a san move wont parse
        break;
      }
      if (move == Move::NO_MOVE) break;

      g_positionDB.record(key, san);
      board.makeMove(move);
    }
  });

  g_trie.prune(3);
  g_openingHash.prune(3);
  g_positionDB.prune(2);

  g_positionDB.printStats();
  cout << "Opening database ready.\n";
}

string lookupPositionDBMove(const string& fen) { return g_positionDB.bestMove(fenToKey(fen)); }
