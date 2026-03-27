#pragma once

#include <string>
#include <utility>
#include <vector>

/** Stream PGN games; tally next-move SAN after prefix. Stops at time limit or 90% confidence (min games). */
struct PgnScanResult {
  std::vector<std::pair<std::string, int>> rankedNext;
  int gamesWithNextMove = 0;
  int gamesVisited = 0;
  double elapsedMs = 0;
  bool stoppedByConfidence = false;
  bool stoppedByTime = false;
  bool fileMissing = false;
};

PgnScanResult scanPgnForNextMoves(const std::string& pgnPath, const std::vector<std::string>& prefixMoves,
                                  double timeLimitMs, double confidence, int minGamesForConfidence);
