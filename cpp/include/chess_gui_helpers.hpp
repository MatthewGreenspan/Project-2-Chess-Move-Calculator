#pragma once

#include "app.hpp"

using namespace std;
namespace chess_gui {

void boardToArray(const Board& board, array<Piece, 64>& out);
string boardToFen(const array<Piece, 64>& pieces, Color toMove);
string moveToPlainEnglish(const Board& board, const Move& move);

bool hasValidKingCount(const Board& board);
Board getBoardForPieceMoves(const Board& board, int squareIndex);
void getMovesForPiece(const Board& board, int squareIndex, Movelist& out);

void updateLegalMoves(App& app);

}  // namespace chess_gui
