#pragma once

#include "string_hash_map.hpp"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

/** Prefix-keyed map: key = move sequence joined with "|", value = next-move SAN → frequency. No std::unordered_map. */
class OpeningHashMap {
  StringHashMap<std::vector<std::pair<std::string, int>>> prefixToNext_;

  static std::string joinKey(const std::vector<std::string>& moves) {
    std::string k;
    for (std::size_t i = 0; i < moves.size(); ++i) {
      if (i) k += "|";
      k += moves[i];
    }
    return k;
  }

  static void bump(std::vector<std::pair<std::string, int>>& inner, const std::string& mv) {
    for (auto& p : inner) {
      if (p.first == mv) {
        ++p.second;
        return;
      }
    }
    inner.push_back({mv, 1});
  }

 public:
  void insertGame(const std::vector<std::string>& moves) {
    std::string prefix;
    for (const auto& mv : moves) {
      bump(prefixToNext_[prefix], mv);
      if (!prefix.empty()) prefix += "|";
      prefix += mv;
    }
  }

  std::vector<std::pair<std::string, int>> getRankedMoves(const std::vector<std::string>& playedMoves, int maxMoves) {
    std::string k = joinKey(playedMoves);
    const auto* inner = prefixToNext_.find(k);
    if (!inner || inner->empty()) return {};
    std::vector<std::pair<std::string, int>> candidates = *inner;
    std::sort(candidates.begin(), candidates.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    if ((int)candidates.size() > maxMoves) candidates.resize((std::size_t)maxMoves);
    return candidates;
  }

  void prune(int minCount) {
    std::vector<std::string> dropKeys;
    prefixToNext_.forEach([&](const std::string& key, std::vector<std::pair<std::string, int>>& inner) {
      std::vector<std::pair<std::string, int>> kept;
      for (const auto& p : inner) {
        if (p.second >= minCount) kept.push_back(p);
      }
      inner = std::move(kept);
      if (inner.empty()) dropKeys.push_back(key);
    });
    for (const auto& k : dropKeys) prefixToNext_.erase(k);
  }
};
