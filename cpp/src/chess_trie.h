#pragma once

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

struct TrieNode {
  std::string move;
  int count = 0;
  std::vector<std::pair<std::string, TrieNode*>> children;

  explicit TrieNode(std::string m = "") : move(std::move(m)) {}
};

class ChessTrie {
  TrieNode* root;

  static TrieNode* findChild(TrieNode* node, const std::string& m) {
    if (!node) return nullptr;
    for (auto& p : node->children)
      if (p.first == m) return p.second;
    return nullptr;
  }

  static TrieNode* getOrCreateChild(TrieNode* node, const std::string& m) {
    if (TrieNode* c = findChild(node, m)) return c;
    TrieNode* n = new TrieNode(m);
    node->children.push_back({m, n});
    return n;
  }

  void deleteNode(TrieNode* node) {
    if (!node) return;
    for (auto& p : node->children) deleteNode(p.second);
    delete node;
  }

  void pruneNode(TrieNode* node, int minCount) {
    if (!node) return;
    std::vector<std::string> toDelete;
    for (const auto& p : node->children) {
      if (p.second->count < minCount) toDelete.push_back(p.first);
    }
    for (const std::string& move : toDelete) {
      for (std::size_t i = 0; i < node->children.size(); ++i) {
        if (node->children[i].first == move) {
          deleteNode(node->children[i].second);
          node->children[i] = std::move(node->children.back());
          node->children.pop_back();
          break;
        }
      }
    }
    for (auto& p : node->children) pruneNode(p.second, minCount);
  }

 public:
  ChessTrie() : root(new TrieNode()) {}

  ~ChessTrie() { deleteNode(root); }

  void insertGame(const std::vector<std::string>& moves) {
    TrieNode* curr = root;
    for (const std::string& move : moves) {
      curr = getOrCreateChild(curr, move);
      curr->count++;
    }
  }

  /** Current node after following played moves, or nullptr if prefix missing. */
  TrieNode* nodeAfterPrefix(const std::vector<std::string>& playedMoves) const {
    TrieNode* curr = root;
    for (const std::string& move : playedMoves) {
      curr = findChild(curr, move);
      if (!curr) return nullptr;
    }
    return curr;
  }

  std::string getBestMove(const std::vector<std::string>& playedMoves) {
    TrieNode* curr = nodeAfterPrefix(playedMoves);
    if (!curr) return "";
    std::string bestMove;
    int maxCount = 0;
    for (const auto& p : curr->children) {
      if (p.second->count > maxCount) {
        maxCount = p.second->count;
        bestMove = p.first;
      }
    }
    return bestMove;
  }

  std::vector<std::pair<std::string, int>> getRankedMoves(const std::vector<std::string>& playedMoves, int maxMoves) {
    TrieNode* curr = nodeAfterPrefix(playedMoves);
    if (!curr) return {};
    std::vector<std::pair<std::string, int>> candidates;
    for (const auto& p : curr->children) candidates.push_back({p.first, p.second->count});
    std::sort(candidates.begin(), candidates.end(),
              [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
                return a.second > b.second;
              });
    if ((int)candidates.size() > maxMoves) candidates.resize((std::size_t)maxMoves);
    return candidates;
  }

  void prune(int minCount = 5) { pruneNode(root, minCount); }
};
