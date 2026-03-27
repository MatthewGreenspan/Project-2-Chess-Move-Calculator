#ifndef STOCKFISH_HPP
#define STOCKFISH_HPP
#include <string>

std::string getBestMoveFromStockfish(const std::string& fen, int movetimeMs = 500);

/** Local Stockfish: evaluate position (side-to-move POV). Returns false if binary missing or parse failed. */
bool stockfishEvalPosition(const std::string& fen, int& cpOut, bool& mateOut, int& mateInOut);

/** Lichess cloud-eval API (needs curl + network). Parses first PV cp/mate. */
bool lichessCloudEvalPosition(const std::string& fen, int& cpOut, bool& mateOut, int& mateInOut);
#endif
