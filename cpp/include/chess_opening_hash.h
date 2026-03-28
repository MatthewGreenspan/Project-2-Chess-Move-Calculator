#pragma once

#include "string_hash_map.hpp"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

using namespace std;
/** Prefix-keyed map: key = move sequence joined with "|", value = next-move SAN → frequency. No unordered_map. */
class OpeningHashMap {
  StringHashMap<vector<pair<string, int>>> prefixToNext_;

  static string joinKey(const vector<string>& moves) {
    string k;
    for (size_t i = 0; i < moves.size(); ++i) {
      if (i) k += "|";
      k += moves[i];
    }
    return k;
  }

  static void bump(vector<pair<string, int>>& inner, const string& mv) {
    for (auto& p : inner) {
      if (p.first == mv) {
        ++p.second;
        return;
      }
    }
    inner.push_back({mv, 1});
  }

 public:
  void insertGame(const vector<string>& moves) {
    string prefix;
    for (const auto& mv : moves) {
      bump(prefixToNext_[prefix], mv);
      if (!prefix.empty()) prefix += "|";
      prefix += mv;
    }
  }

  vector<pair<string, int>> getRankedMoves(const vector<string>& playedMoves, int maxMoves) {
    string k = joinKey(playedMoves);
    const auto* inner = prefixToNext_.find(k);
    if (!inner || inner->empty()) return {};
    vector<pair<string, int>> candidates = *inner;
    sort(candidates.begin(), candidates.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    if ((int)candidates.size() > maxMoves) candidates.resize((size_t)maxMoves);
    return candidates;
  }

  void prune(int minCount) {
    vector<string> dropKeys;
    prefixToNext_.forEach([&](const string& key, vector<pair<string, int>>& inner) {
      vector<pair<string, int>> kept;
      for (const auto& p : inner) {
        if (p.second >= minCount) kept.push_back(p);
      }
      inner = static_cast<vector<pair<string, int>>&&>(kept);
      if (inner.empty()) dropKeys.push_back(key);
    });
    for (const auto& k : dropKeys) prefixToNext_.erase(k);
  }
};
