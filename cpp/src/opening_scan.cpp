#include "opening_scan.hpp"
#include "pgn_parser.h"

#include <algorithm>
#include <chrono>
#include <string>

using namespace std;
static void bump(vector<pair<string, int>>& votes, const string& san) {
  for (auto& p : votes) {
    if (p.first == san) {
      ++p.second;
      return;
    }
  }
  votes.push_back({san, 1});
}

static bool prefixMatches(const vector<string>& gameMoves, const vector<string>& prefix) {
  if (gameMoves.size() < prefix.size()) return false;
  for (size_t i = 0; i < prefix.size(); ++i) {
    if (gameMoves[i] != prefix[i]) return false;
  }
  return true;
}

static void sortVotesDesc(vector<pair<string, int>>& v) {
  sort(v.begin(), v.end(),
            [](const pair<string, int>& a, const pair<string, int>& b) {
              return a.second > b.second;
            });
}

static bool confidenceReached(const vector<pair<string, int>>& votes, int totalGames,
                              double confidence, int minGames) {
  if (totalGames < minGames || votes.empty()) return false;
  int best = votes[0].second;
  return static_cast<double>(best) >= confidence * static_cast<double>(totalGames);
}

PgnScanResult scanPgnForNextMoves(const string& pgnPath, const vector<string>& prefixMoves,
                                  double timeLimitMs, double confidence, int minGamesForConfidence) {
  PgnScanResult out;
  if (pgnPath.empty()) {
    out.fileMissing = true;
    return out;
  }

  using clock = chrono::steady_clock;
  auto t0 = clock::now();

  vector<pair<string, int>> votes;
  PGNParser parser(1800, 2500, 40);

  parser.parseWithEarlyExit(pgnPath, [&](const GameData& g) -> bool {
    ++out.gamesVisited;
    if (!prefixMatches(g.moves, prefixMoves)) return true;
    if (g.moves.size() <= prefixMoves.size()) return true;

    const string& next = g.moves[prefixMoves.size()];
    bump(votes, next);
    ++out.gamesWithNextMove;
    sortVotesDesc(votes);

    auto t1 = clock::now();
    out.elapsedMs =
        chrono::duration<double, milli>(t1 - t0).count();
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
  out.elapsedMs = chrono::duration<double, milli>(tEnd - t0).count();
  sortVotesDesc(votes);
  out.rankedNext = static_cast<vector<pair<string, int>>&&>(votes);
  return out;
}
