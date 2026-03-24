#pragma once

#include "app.hpp"

namespace chess_gui {

void boardToArray(const Board& board, std::array<Piece, 64>& out);
std::string boardToFen(const std::array<Piece, 64>& pieces, Color toMove);
std::string moveToPlainEnglish(const Board& board, const Move& move);

bool hasValidKingCount(const Board& board);
Board getBoardForPieceMoves(const Board& board, int squareIndex);
void getMovesForPiece(const Board& board, int squareIndex, Movelist& out);

void updateLegalMoves(App& app);

}  // namespace chess_gui
