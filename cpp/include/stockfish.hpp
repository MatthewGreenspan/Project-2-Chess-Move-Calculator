#ifndef STOCKFISH_HPP
#define STOCKFISH_HPP
#include <string>

using namespace std;

// asks stockfish for the best move in uci form
string getBestMoveFromStockfish(const string& fen, int movetimeMs = 500);

bool stockfishEvalPosition(const string& fen, int& cpOut, bool& mateOut, int& mateInOut);

bool lichessCloudEvalPosition(const string& fen, int& cpOut, bool& mateOut, int& mateInOut);
#endif
