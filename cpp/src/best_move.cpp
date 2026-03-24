#include "best_move.hpp"
#include "chess_gui_helpers.hpp"
#include "opening_db.hpp"
#include "opening_suggestion.hpp"
#include "stockfish.hpp"

using namespace chess;
using namespace chess_gui;

namespace {

constexpr int kOpeningPliesSkipKingWalk = 28;

}  // namespace

void clearBestMoveDisplay(App& app) {
  app.bestMoveFrom = -1;
  app.bestMoveTo = -1;
  app.showBestMoveArrow = false;
}

void updateBestMove(App& app, bool force) {
  std::string fen = app.board.getFen();
  if (!force && fen == app.lastAnalyzedFen) return;
  app.lastAnalyzedFen = fen;
  app.bestMoveUci = "";
  app.bestMoveSan = "";
  app.bestMoveEnglish = "";
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

  if (!app.movesPlayed.empty()) {
    auto ranked = g_trie.getRankedMoves(app.movesPlayed, 16);
    const bool inOpening = static_cast<int>(app.movesPlayed.size()) < kOpeningPliesSkipKingWalk;
    for (const auto& entry : ranked) {
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
}
