#include "best_move.hpp"
#include "chess_gui_helpers.hpp"
#include "opening_db.hpp"
#include "opening_suggestion.hpp"
#include "stockfish.hpp"
#include "fen_parser.hpp"

#include <chrono>
#include <cstdio>
#include <string>

using namespace chess;
using namespace chess_gui;

namespace {

constexpr int kOpeningPliesSkipKingWalk = 28;
constexpr int kMaxOpeningTriePlies = 40;

static std::string formatMoveGrade(const Board& rootBoard, Move m) {
  Board b = rootBoard;
  b.makeMove(m);
  std::string fenAfter = b.getFen();
  int cp = 0;
  bool mate = false;
  int mateIn = 0;
  if (!stockfishEvalPosition(fenAfter, cp, mate, mateIn)) return "SF grade: (need local Stockfish)";

  if (mate) {
    if (mateIn > 0)
      return "SF d14: they mate in " + std::to_string(mateIn) + " (bad for last move)";
    return "SF d14: you mate in " + std::to_string(-mateIn) + " (good for last move)";
  }
  int moverCp = -cp;
  char buf[112];
  std::snprintf(buf, sizeof(buf), "SF d14: %+0.2f pawns (last mover ~cp %d)", moverCp / 100.0, moverCp);
  return std::string(buf);
}

}  // namespace

void clearBestMoveDisplay(App& app) {
  app.bestMoveFrom = -1;
  app.bestMoveTo = -1;
  app.showBestMoveArrow = false;
  app.openingLookupCompare.clear();
  app.moveGradeLine.clear();
}

void updateBestMove(App& app, bool force) {
  std::string fen = app.board.getFen();
  if (!force && fen == app.lastAnalyzedFen) return;
  app.lastAnalyzedFen = fen;
  app.bestMoveUci = "";
  app.bestMoveSan = "";
  app.bestMoveEnglish = "";
  app.openingLookupCompare.clear();
  app.moveGradeLine.clear();
  app.bestMoveFrom = -1;
  app.bestMoveTo = -1;

  if (!hasValidKingCount(app.board)) {
    app.bestMoveEnglish = "Invalid position (need 1 king each)";
    return;
  }
  Movelist moves;
  movegen::legalmoves(moves, app.board);
  if (moves.size() == 0) {
    app.bestMoveEnglish = "Game over";
    return;
  }

  {
    std::string bookMove = lookupPositionDBMove(fen);
    if (!bookMove.empty()) {
      for (size_t i = 0; i < moves.size(); i++) {
        if (uci::moveToSan(app.board, moves[i]) == bookMove) {
          app.bestMoveSan = bookMove;
          app.bestMoveEnglish = "Book (position DB)";
          app.bestMoveFrom = moves[i].from().index();
          app.bestMoveTo = moves[i].to().index();
          app.showBestMoveArrow = true;
          app.moveGradeLine = formatMoveGrade(app.board, moves[i]);
          return;
        }
      }
    }
  }

  if (!app.movesPlayed.empty() && static_cast<int>(app.movesPlayed.size()) < kMaxOpeningTriePlies) {
    using clock = std::chrono::high_resolution_clock;
    auto t0 = clock::now();
    auto rankedTrie = g_trie.getRankedMoves(app.movesPlayed, 16);
    auto t1 = clock::now();
    auto t2 = clock::now();
    auto rankedHash = g_openingHash.getRankedMoves(app.movesPlayed, 16);
    auto t3 = clock::now();
    long long nsTrie = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    long long nsHash = std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t2).count();
    char cmp[160];
    const char* faster = nsTrie < nsHash ? "Trie faster" : (nsHash < nsTrie ? "Hash faster" : "tie");
    std::snprintf(cmp, sizeof(cmp), "Trie %lld ns | Hash %lld ns — %s", (long long)nsTrie, (long long)nsHash, faster);
    app.openingLookupCompare = cmp;

    const bool inOpening = static_cast<int>(app.movesPlayed.size()) < kOpeningPliesSkipKingWalk;
    for (const auto& entry : rankedTrie) {
      const std::string& san = entry.first;
      if (san.empty()) continue;
      if (inOpening && sanIsNonCastlingKingMove(san)) continue;
      for (size_t i = 0; i < moves.size(); i++) {
        if (uci::moveToSan(app.board, moves[i]) == san) {
          app.bestMoveSan = san;
          app.bestMoveEnglish = "Most popular (opening DB)";
          app.bestMoveFrom = moves[i].from().index();
          app.bestMoveTo = moves[i].to().index();
          app.showBestMoveArrow = true;
          app.moveGradeLine = formatMoveGrade(app.board, moves[i]);
          return;
        }
      }
    }
  }

  std::string uci = getBestMoveFromStockfish(fen, 400);
  Move m = Move::NO_MOVE;
  if (!uci.empty()) m = uci::uciToMove(app.board, uci);
  if (m == Move::NO_MOVE) m = moves[0];

  app.bestMoveUci = uci;
  app.bestMoveSan = uci::moveToSan(app.board, m);
  app.bestMoveEnglish = moveToPlainEnglish(app.board, m);
  app.bestMoveFrom = m.from().index();
  app.bestMoveTo = m.to().index();
  app.showBestMoveArrow = true;
  app.moveGradeLine = formatMoveGrade(app.board, m);
}
