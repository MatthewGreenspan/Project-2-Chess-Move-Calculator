#include "chess_gui_helpers.hpp"
#include "gui_constants.hpp"

namespace chess_gui {

void boardToArray(const Board& board, std::array<Piece, 64>& out) {
  for (int i = 0; i < 64; i++) out[i] = board.at<Piece>(Square(i));
}

std::string boardToFen(const std::array<Piece, 64>& pieces, Color toMove) {
  std::string fen;
  for (int r = 7; r >= 0; r--) {
    int empty = 0;
    for (int f = 0; f < 8; f++) {
      Piece p = pieces[r * 8 + f];
      if (p == Piece::NONE)
        empty++;
      else {
        if (empty > 0) {
          fen += std::to_string(empty);
          empty = 0;
        }
        fen += static_cast<std::string>(p);
      }
    }
    if (empty > 0) fen += std::to_string(empty);
    if (r > 0) fen += '/';
  }
  fen += toMove == Color::WHITE ? " w" : " b";
  fen += " KQkq - 0 1";
  return fen;
}

std::string moveToPlainEnglish(const Board& board, const Move& move) {
  if (move == Move::NO_MOVE) return "";
  PieceType pt = board.at<PieceType>(move.from());
  std::string toSq = static_cast<std::string>(move.to());
  bool capture = board.isCapture(move);

  if (move.typeOf() == Move::CASTLING) {
    return (move.to() > move.from()) ? "Kingside castling" : "Queenside castling";
  }
  if (move.typeOf() == Move::ENPASSANT) {
    return "Pawn captures en passant on " + toSq;
  }
  if (move.typeOf() == Move::PROMOTION) {
    std::string promo = PIECE_NAMES_CAP[static_cast<int>(move.promotionType())];
    return "Pawn promotes to " + promo + " on " + toSq;
  }

  std::string piece = PIECE_NAMES_CAP[static_cast<int>(pt)];
  if (capture)
    return piece + " captures on " + toSq;
  return piece + " to " + toSq;
}

bool hasValidKingCount(const Board& board) {
  auto wk = board.pieces(PieceType::KING, Color::WHITE);
  auto bk = board.pieces(PieceType::KING, Color::BLACK);
  return wk.count() == 1 && bk.count() == 1;
}

Board getBoardForPieceMoves(const Board& board, int squareIndex) {
  Piece p = board.at<Piece>(Square(squareIndex));
  if (p == Piece::NONE) return board;
  if (p.color() == board.sideToMove()) return board;
  Board b = board;
  std::string fen = board.getFen();
  size_t pos = fen.find(' ');
  if (pos != std::string::npos && pos + 2 <= fen.size())
    fen[pos + 1] = (fen[pos + 1] == 'w') ? 'b' : 'w';
  b.setFen(fen);
  return b;
}

void getMovesForPiece(const Board& board, int squareIndex, Movelist& out) {
  out.clear();
  if (!hasValidKingCount(board)) return;
  Piece p = board.at<Piece>(Square(squareIndex));
  if (p == Piece::NONE) return;
  Board b = getBoardForPieceMoves(board, squareIndex);
  movegen::legalmoves(out, b);
}

void updateLegalMoves(App& app) {
  app.legalMoveSquares.clear();
  if (app.selectedSquare < 0) return;
  Movelist moves;
  getMovesForPiece(app.board, app.selectedSquare, moves);
  for (size_t i = 0; i < moves.size(); i++) {
    if (moves[i].from() == Square(app.selectedSquare))
      app.legalMoveSquares.push_back(moves[i].to());
  }
}

}  // namespace chess_gui
