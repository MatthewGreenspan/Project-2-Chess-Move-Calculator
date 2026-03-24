#include "best_move.hpp"
#include "chess_gui_helpers.hpp"
#include "opening_db.hpp"
#include "stockfish.hpp"

using namespace chess;
using namespace chess_gui;

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
    std::string trieBest = g_trie.getBestMove(app.movesPlayed);
    if (!trieBest.empty()) {
      app.bestMoveSan = trieBest;
      app.bestMoveEnglish = "Most popular in your rating range";
      for (size_t i = 0; i < moves.size(); i++) {
        if (uci::moveToSan(app.board, moves[i]) == trieBest) {
          app.bestMoveFrom = moves[i].from().index();
          app.bestMoveTo = moves[i].to().index();
          app.showBestMoveArrow = true;
          break;
        }
      }
      return;
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
