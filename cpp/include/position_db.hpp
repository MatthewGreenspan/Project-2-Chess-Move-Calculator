#pragma once

#include "string_hash_map.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
struct MoveEntry {
  string san;
  int count = 0;
};

class PositionDB {
 public:
  void record(const string& fenKey, const string& san) {
    vector<MoveEntry>& entries = table_[fenKey];
    for (auto& e : entries) {
      if (e.san == san) {
        ++e.count;
        return;
      }
    }
    entries.push_back({san, 1});
  }

  void prune(int minCount) {
    vector<string> dropKeys;
    table_.forEach([&](const string& key, vector<MoveEntry>& entries) {
      entries.erase(remove_if(entries.begin(), entries.end(),
                                   [minCount](const MoveEntry& e) { return e.count < minCount; }),
                    entries.end());
      sort(entries.begin(), entries.end(),
                [](const MoveEntry& a, const MoveEntry& b) { return a.count > b.count; });
      if (entries.empty()) dropKeys.push_back(key);
    });
    for (const auto& k : dropKeys) table_.erase(k);
  }

  const vector<MoveEntry>* lookup(const string& fenKey) const {
    return table_.find(fenKey);
  }

  string bestMove(const string& fenKey) const {
    const auto* entries = lookup(fenKey);
    if (!entries || entries->empty()) return "";
    return (*entries)[0].san;
  }

  size_t size() const { return table_.size(); }

  void printStats() const {
    size_t total = 0;
    table_.forEach([&total](const string&, const vector<MoveEntry>& v) { total += v.size(); });
    cout << "[PositionDB] " << table_.size() << " positions, " << total << " move entries\n";
  }

 private:
  StringHashMap<vector<MoveEntry>> table_;
};

extern PositionDB g_positionDB;
