#pragma once

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

using namespace std;
struct TrieNode {
  string move;
  int count = 0;
  vector<pair<string, TrieNode*>> children;

  explicit TrieNode(string m = "") : move(static_cast<string&&>(m)) {}
};

class ChessTrie {
  TrieNode* root;

  static TrieNode* findChild(TrieNode* node, const string& m) {
    if (!node) return nullptr;
    for (auto& p : node->children)
      if (p.first == m) return p.second;
    return nullptr;
  }

  static TrieNode* getOrCreateChild(TrieNode* node, const string& m) {
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
    vector<string> toDelete;
    for (const auto& p : node->children) {
      if (p.second->count < minCount) toDelete.push_back(p.first);
    }
    for (const string& moveText : toDelete) {
      for (size_t i = 0; i < node->children.size(); ++i) {
        if (node->children[i].first == moveText) {
          deleteNode(node->children[i].second);
          node->children[i] = static_cast<pair<string, TrieNode*>&&>(node->children.back());
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

  void insertGame(const vector<string>& moves) {
    TrieNode* curr = root;
    for (const string& move : moves) {
      curr = getOrCreateChild(curr, move);
      curr->count++;
    }
  }

  /** Current node after following played moves, or nullptr if prefix missing. */
  TrieNode* nodeAfterPrefix(const vector<string>& playedMoves) const {
    TrieNode* curr = root;
    for (const string& move : playedMoves) {
      curr = findChild(curr, move);
      if (!curr) return nullptr;
    }
    return curr;
  }

  string getBestMove(const vector<string>& playedMoves) {
    TrieNode* curr = nodeAfterPrefix(playedMoves);
    if (!curr) return "";
    string bestMove;
    int maxCount = 0;
    for (const auto& p : curr->children) {
      if (p.second->count > maxCount) {
        maxCount = p.second->count;
        bestMove = p.first;
      }
    }
    return bestMove;
  }

  vector<pair<string, int>> getRankedMoves(const vector<string>& playedMoves, int maxMoves) {
    TrieNode* curr = nodeAfterPrefix(playedMoves);
    if (!curr) return {};
    vector<pair<string, int>> candidates;
    for (const auto& p : curr->children) candidates.push_back({p.first, p.second->count});
    sort(candidates.begin(), candidates.end(),
              [](const pair<string, int>& a, const pair<string, int>& b) {
                return a.second > b.second;
              });
    if ((int)candidates.size() > maxMoves) candidates.resize((size_t)maxMoves);
    return candidates;
  }

  void prune(int minCount = 5) { pruneNode(root, minCount); }
};
