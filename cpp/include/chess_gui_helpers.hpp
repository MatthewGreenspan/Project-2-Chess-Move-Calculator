#pragma once

#include "app.hpp"

using namespace std;
namespace chess_gui {

// copies board pieces into the flat array for drawing
void boardToArray(const Board& board, array<Piece, 64>& out);
string boardToFen(const array<Piece, 64>& pieces, Color toMove);
string moveToPlainEnglish(const Board& board, const Move& move);

// makes sure the edited board still has valid kings
bool hasValidKingCount(const Board& board);
Board getBoardForPieceMoves(const Board& board, int squareIndex);
void getMovesForPiece(const Board& board, int squareIndex, Movelist& out);

// refreshes legal move highlights for the selected square
void updateLegalMoves(App& app);

}  
