#pragma once

#include <string>
#include <utility>
#include <vector>

using namespace std;
//Stream PGN games; tally next-move SAN after prefix. Stops at time limit or 90% confidence (min games). 
struct PgnScanResult {
  // ranked next moves we found from the scan
  vector<pair<string, int>> rankedNext;
  int gamesWithNextMove = 0;
  int gamesVisited = 0;
  double elapsedMs = 0;
  bool stoppedByConfidence = false;
  bool stoppedByTime = false;
  bool fileMissing = false;
};

// scans the pgn till time/confidence says stop
PgnScanResult scanPgnForNextMoves(const string& pgnPath, const vector<string>& prefixMoves,
                                  double timeLimitMs, double confidence, int minGamesForConfidence);
