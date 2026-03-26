#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iostream>

struct MoveEntry {
    std::string san;
    int count = 0;
};

class PositionDB {
public:
    void record(const std::string& fenKey, const std::string& san) {
        auto& entries = table_[fenKey];
        for (auto& e : entries) {
            if (e.san == san) { ++e.count; return; }
        }
        entries.push_back({san, 1});
    }

    void prune(int minCount) {
        for (auto it = table_.begin(); it != table_.end(); ) {
            auto& entries = it->second;
            entries.erase(
                std::remove_if(entries.begin(), entries.end(),
                    [minCount](const MoveEntry& e){ return e.count < minCount; }),
                entries.end()
            );
            std::sort(entries.begin(), entries.end(),
                [](const MoveEntry& a, const MoveEntry& b){ return a.count > b.count; });
            if (entries.empty()) it = table_.erase(it);
            else                 ++it;
        }
    }

    const std::vector<MoveEntry>* lookup(const std::string& fenKey) const {
        auto it = table_.find(fenKey);
        if (it == table_.end()) return nullptr;
        return &it->second;
    }

    std::string bestMove(const std::string& fenKey) const {
        const auto* entries = lookup(fenKey);
        if (!entries || entries->empty()) return "";
        return (*entries)[0].san;
    }

    size_t size() const { return table_.size(); }

    void printStats() const {
        size_t total = 0;
        for (auto& [k, v] : table_) total += v.size();
        std::cout << "[PositionDB] " << table_.size()
                  << " positions, " << total << " move entries\n";
    }

private:
    std::unordered_map<std::string, std::vector<MoveEntry>> table_;
};

extern PositionDB g_positionDB;