#include "app_actions.hpp"
#include "best_move.hpp"
#include "chess_gui_helpers.hpp"

using namespace chess;
using namespace chess_gui;

void doDrop(App& app, int toSquare) {
  if (app.dragFrom >= 0 && app.dragFrom < 64) {
    Piece p = app.pieces[app.dragFrom];
    if (p != Piece::NONE && toSquare >= 0 && toSquare < 64) {
      app.pieces[app.dragFrom] = Piece::NONE;
      app.pieces[toSquare] = p;
      std::string fen = boardToFen(app.pieces, app.board.sideToMove());
      app.board.setFen(fen);
      app.selectedSquare = -1;
      app.legalMoveSquares.clear();
    }
    app.dragFrom = -1;
  } else if (app.dragFromPalette >= 0 && toSquare >= 0 && toSquare < 64) {
    int color = app.dragFromPalette < 6 ? 0 : 1;
    static const PieceType PT_MAP[] = {PieceType::KING, PieceType::QUEEN, PieceType::ROOK,
                                       PieceType::BISHOP, PieceType::KNIGHT, PieceType::PAWN};
    PieceType pt = PT_MAP[app.dragFromPalette % 6];
    Piece p(PieceType(pt), color == 0 ? Color::WHITE : Color::BLACK);
    app.pieces[toSquare] = p;
    std::string fen = boardToFen(app.pieces, app.board.sideToMove());
    if (app.board.setFen(fen)) app.selectedSquare = -1;
    app.dragFromPalette = -1;
  }
  updateLegalMoves(app);
  clearBestMoveDisplay(app);
}

void doRemove(App& app) {
  if (app.dragFrom >= 0 && app.dragFrom < 64) {
    app.pieces[app.dragFrom] = Piece::NONE;
    std::string fen = boardToFen(app.pieces, app.board.sideToMove());
    app.board.setFen(fen);
    app.dragFrom = -1;
    app.selectedSquare = -1;
    app.legalMoveSquares.clear();
    clearBestMoveDisplay(app);
  }
}

bool tryLegalMove(App& app, int fromSq, int toSq) {
  Movelist moves;
  getMovesForPiece(app.board, fromSq, moves);
  for (size_t i = 0; i < moves.size(); i++) {
    if (moves[i].from() == Square(fromSq) && moves[i].to() == Square(toSq)) {
      Board b = getBoardForPieceMoves(app.board, fromSq);

      app.movesPlayed.push_back(uci::moveToSan(b, moves[i]));

      b.makeMove(moves[i]);
      app.board = b;
      boardToArray(app.board, app.pieces);
      app.selectedSquare = -1;
      app.legalMoveSquares.clear();
      clearBestMoveDisplay(app);
      return true;
    }
  }
  return false;
}
