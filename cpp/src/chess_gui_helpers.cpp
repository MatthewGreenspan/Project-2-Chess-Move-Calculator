#include "chess_gui_helpers.hpp"
#include "gui_constants.hpp"

using namespace std;
namespace chess_gui {

// copies board state into the render array
void boardToArray(const Board& board, array<Piece, 64>& out) {
  for (int i = 0; i < 64; i++) out[i] = board.at<Piece>(Square(i));
}

// builds a simple fen from the sandbox piece array
string boardToFen(const array<Piece, 64>& pieces, Color toMove) {
  string fen;
  for (int r = 7; r >= 0; r--) {
    int empty = 0;
    for (int f = 0; f < 8; f++) {
      Piece p = pieces[r * 8 + f];
      if (p == Piece::NONE)
        empty++;
      else {
        if (empty > 0) {
          fen += to_string(empty);
          empty = 0;
        }
        fen += static_cast<string>(p);
      }
    }
    if (empty > 0) fen += to_string(empty);
    if (r > 0) fen += '/';
  }
  fen += toMove == Color::WHITE ? " w" : " b";
  fen += " KQkq - 0 1";
  return fen;
}

// turns a move into short human readable text
string moveToPlainEnglish(const Board& board, const Move& move) {
  if (move == Move::NO_MOVE) return "";
  PieceType pt = board.at<PieceType>(move.from());
  string toSq = static_cast<string>(move.to());
  bool capture = board.isCapture(move);

  if (move.typeOf() == Move::CASTLING) {
    return (move.to() > move.from()) ? "Kingside castling" : "Queenside castling";
  }
  if (move.typeOf() == Move::ENPASSANT) {
    return "Pawn captures en passant on " + toSq;
  }
  if (move.typeOf() == Move::PROMOTION) {
    string promo = PIECE_NAMES_CAP[static_cast<int>(move.promotionType())];
    return "Pawn promotes to " + promo + " on " + toSq;
  }

  string piece = PIECE_NAMES_CAP[static_cast<int>(pt)];
  if (capture)
    return piece + " captures on " + toSq;
  return piece + " to " + toSq;
}

// sandbox boards still need exactly one king each
bool hasValidKingCount(const Board& board) {
  auto wk = board.pieces(PieceType::KING, Color::WHITE);
  auto bk = board.pieces(PieceType::KING, Color::BLACK);
  return wk.count() == 1 && bk.count() == 1;
}

// flips side to move if we need enemy piece legal moves
Board getBoardForPieceMoves(const Board& board, int squareIndex) {
  Piece p = board.at<Piece>(Square(squareIndex));
  if (p == Piece::NONE) return board;
  if (p.color() == board.sideToMove()) return board;
  Board b = board;
  string fen = board.getFen();
  size_t pos = fen.find(' ');
  if (pos != string::npos && pos + 2 <= fen.size())
    fen[pos + 1] = (fen[pos + 1] == 'w') ? 'b' : 'w';
  b.setFen(fen);
  return b;
}

// gets legal moves for one square only
void getMovesForPiece(const Board& board, int squareIndex, Movelist& out) {
  out.clear();
  if (!hasValidKingCount(board)) return;
  Piece p = board.at<Piece>(Square(squareIndex));
  if (p == Piece::NONE) return;
  Board b = getBoardForPieceMoves(board, squareIndex);
  movegen::legalmoves(out, b);
}

// refreshes the highlighted legal target squares
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

} 
