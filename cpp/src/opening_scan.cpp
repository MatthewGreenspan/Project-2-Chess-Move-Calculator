#include "opening_scan.hpp"
#include "pgn_parser.h"

#include <algorithm>
#include <chrono>
#include <string>

static void bump(std::vector<std::pair<std::string, int>>& votes, const std::string& san) {
  for (auto& p : votes) {
    if (p.first == san) {
      ++p.second;
      return;
    }
  }
  votes.push_back({san, 1});
}

static bool prefixMatches(const std::vector<std::string>& gameMoves, const std::vector<std::string>& prefix) {
  if (gameMoves.size() < prefix.size()) return false;
  for (std::size_t i = 0; i < prefix.size(); ++i) {
    if (gameMoves[i] != prefix[i]) return false;
  }
  return true;
}

static void sortVotesDesc(std::vector<std::pair<std::string, int>>& v) {
  std::sort(v.begin(), v.end(),
            [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
              return a.second > b.second;
            });
}

static bool confidenceReached(const std::vector<std::pair<std::string, int>>& votes, int totalGames,
                              double confidence, int minGames) {
  if (totalGames < minGames || votes.empty()) return false;
  int best = votes[0].second;
  return static_cast<double>(best) >= confidence * static_cast<double>(totalGames);
}

PgnScanResult scanPgnForNextMoves(const std::string& pgnPath, const std::vector<std::string>& prefixMoves,
                                  double timeLimitMs, double confidence, int minGamesForConfidence) {
  PgnScanResult out;
  if (pgnPath.empty()) {
    out.fileMissing = true;
    return out;
  }

  using clock = std::chrono::steady_clock;
  auto t0 = clock::now();

  std::vector<std::pair<std::string, int>> votes;
  PGNParser parser(1800, 2500, 40);

  parser.parseWithEarlyExit(pgnPath, [&](const GameData& g) -> bool {
    ++out.gamesVisited;
    if (!prefixMatches(g.moves, prefixMoves)) return true;
    if (g.moves.size() <= prefixMoves.size()) return true;

    const std::string& next = g.moves[prefixMoves.size()];
    bump(votes, next);
    ++out.gamesWithNextMove;
    sortVotesDesc(votes);

    auto t1 = clock::now();
    out.elapsedMs =
        std::chrono::duration<double, std::milli>(t1 - t0).count();
    if (out.elapsedMs >= timeLimitMs) {
      out.stoppedByTime = true;
      return false;
    }
    if (confidenceReached(votes, out.gamesWithNextMove, confidence, minGamesForConfidence)) {
      out.stoppedByConfidence = true;
      return false;
    }
    return true;
  });

  auto tEnd = clock::now();
  out.elapsedMs = std::chrono::duration<double, std::milli>(tEnd - t0).count();
  sortVotesDesc(votes);
  out.rankedNext = std::move(votes);
  return out;
}
