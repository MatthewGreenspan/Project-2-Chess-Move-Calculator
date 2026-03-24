#pragma once
#include <string>
#include <utility>
#include <unordered_map>
#include <vector>

using namespace std;

struct TrieNode {
  string move;
  int count = 0;
  unordered_map<string, TrieNode*> children;

  explicit TrieNode(string m = "") : move(std::move(m)) {}
};

class ChessTrie {
  TrieNode* root;

  void deleteNode(TrieNode* node) {
    if (!node) return;
    for (auto& [_, child] : node->children) deleteNode(child);
    delete node;
  }

  void pruneNode(TrieNode* node, int minCount) {
    if (!node) return;
    vector<string> toDelete;
    for (auto& [move, child] : node->children) {
      if (child->count < minCount) toDelete.push_back(move);
    }
    for (const string& move : toDelete) {
      deleteNode(node->children[move]);
      node->children.erase(move);
    }
    for (auto& [_, child] : node->children) pruneNode(child, minCount);
  }

public:
  ChessTrie() : root(new TrieNode()) {}

  ~ChessTrie() { deleteNode(root); }

  void insertGame(const vector<string>& moves) {
    TrieNode* curr = root;
    for (const string& move : moves) {
      if (curr->children.find(move) == curr->children.end())
        curr->children[move] = new TrieNode(move);
      curr = curr->children[move];
      curr->count++;
    }
  }

  string getBestMove(const vector<string>& playedMoves) {
    TrieNode* curr = root;
    for (const string& move : playedMoves) {
      auto it = curr->children.find(move);
      if (it == curr->children.end()) return "";
      curr = it->second;
    }
    string bestMove;
    int maxCount = 0;
    for (auto& [move, node] : curr->children) {
      if (node->count > maxCount) {
        maxCount = node->count;
        bestMove = move;
      }
    }
    return bestMove;
  }

  void prune(int minCount = 5) { pruneNode(root, minCount); }
};
