#pragma once

#include <algorithm>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

/** Prefix-keyed hash map: key = move sequence joined with "|", value = next-move SAN -> frequency. */
class OpeningHashMap {
  std::unordered_map<std::string, std::unordered_map<std::string, int>> prefixToNext_;

  static std::string joinKey(const std::vector<std::string>& moves) {
    std::string k;
    for (size_t i = 0; i < moves.size(); ++i) {
      if (i) k += "|";
      k += moves[i];
    }
    return k;
  }

public:
  void insertGame(const std::vector<std::string>& moves) {
    std::string prefix;
    for (const auto& mv : moves) {
      prefixToNext_[prefix][mv]++;
      if (!prefix.empty()) prefix += "|";
      prefix += mv;
    }
  }

  std::vector<std::pair<std::string, int>> getRankedMoves(const std::vector<std::string>& playedMoves, int maxMoves) {
    std::string k = joinKey(playedMoves);
    auto it = prefixToNext_.find(k);
    if (it == prefixToNext_.end()) return {};
    std::vector<std::pair<std::string, int>> candidates;
    for (auto& [m, c] : it->second) candidates.push_back({m, c});
    std::sort(candidates.begin(), candidates.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    if ((int)candidates.size() > maxMoves) candidates.resize(maxMoves);
    return candidates;
  }

  void prune(int minCount) {
    for (auto& [prefix, inner] : prefixToNext_) {
      std::vector<std::string> dels;
      for (auto& [m, c] : inner) {
        if (c < minCount) dels.push_back(m);
      }
      for (const auto& m : dels) inner.erase(m);
    }
  }
};
