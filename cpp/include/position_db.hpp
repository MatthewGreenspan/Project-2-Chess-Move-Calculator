#pragma once

#include "string_hash_map.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

struct MoveEntry {
  std::string san;
  int count = 0;
};

class PositionDB {
 public:
  void record(const std::string& fenKey, const std::string& san) {
    std::vector<MoveEntry>& entries = table_[fenKey];
    for (auto& e : entries) {
      if (e.san == san) {
        ++e.count;
        return;
      }
    }
    entries.push_back({san, 1});
  }

  void prune(int minCount) {
    std::vector<std::string> dropKeys;
    table_.forEach([&](const std::string& key, std::vector<MoveEntry>& entries) {
      entries.erase(std::remove_if(entries.begin(), entries.end(),
                                   [minCount](const MoveEntry& e) { return e.count < minCount; }),
                    entries.end());
      std::sort(entries.begin(), entries.end(),
                [](const MoveEntry& a, const MoveEntry& b) { return a.count > b.count; });
      if (entries.empty()) dropKeys.push_back(key);
    });
    for (const auto& k : dropKeys) table_.erase(k);
  }

  const std::vector<MoveEntry>* lookup(const std::string& fenKey) const {
    return table_.find(fenKey);
  }

  std::string bestMove(const std::string& fenKey) const {
    const auto* entries = lookup(fenKey);
    if (!entries || entries->empty()) return "";
    return (*entries)[0].san;
  }

  std::size_t size() const { return table_.size(); }

  void printStats() const {
    std::size_t total = 0;
    table_.forEach([&total](const std::string&, const std::vector<MoveEntry>& v) { total += v.size(); });
    std::cout << "[PositionDB] " << table_.size() << " positions, " << total << " move entries\n";
  }

 private:
  StringHashMap<std::vector<MoveEntry>> table_;
};

extern PositionDB g_positionDB;
