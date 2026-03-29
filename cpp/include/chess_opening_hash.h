#pragma once

#include "string_hash_map.hpp"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

using namespace std;
/** Prefix-keyed map: key = move sequence joined with "|", value = next-move SAN → frequency. No unordered_map. */
class OpeningHashMap {
  // holds the next move counts for each prefix
  StringHashMap<vector<pair<string, int>>> prefixToNext_;

  // makes one string key so lookup is easier
  static string joinKey(const vector<string>& moves) {
    // temp key while we join every move together
    string k;
    for (size_t i = 0; i < moves.size(); ++i) {
      if (i) k += "|";
      k += moves[i];
    }
    return k;
  }

  // bumps the count if move exists already, else adds it
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
  // inserts one whole game into the hash map
  void insertGame(const vector<string>& moves) {
    // keeps track of the prefix were currently on
    string prefix;
    for (const auto& mv : moves) {
      bump(prefixToNext_[prefix], mv);
      if (!prefix.empty()) prefix += "|";
      prefix += mv;
    }
  }

  // gets the most common next moves for a played line
  vector<pair<string, int>> getRankedMoves(const vector<string>& playedMoves, int maxMoves) {
    string k = joinKey(playedMoves);
    const auto* inner = prefixToNext_.find(k);
    if (!inner || inner->empty()) return {};
    // copy it out so we can sort without changing stored data
    vector<pair<string, int>> candidates = *inner;
    sort(candidates.begin(), candidates.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    if ((int)candidates.size() > maxMoves) candidates.resize((size_t)maxMoves);
    return candidates;
  }

  // removes weak entries so the table dont get too noisy
  void prune(int minCount) {
    // temp list for keys we wanna delete after looping
    vector<string> dropKeys;
    prefixToNext_.forEach([&](const string& key, vector<pair<string, int>>& inner) {
      // holds only the move counts we decided to keep
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
