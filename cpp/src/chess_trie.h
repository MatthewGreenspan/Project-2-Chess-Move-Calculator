#pragma once
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// -------------------------------------------------------
// TrieNode
// Each node in the trie represents a chess move.
// A path from root to any node spells out a sequence
// of moves, e.g. root -> "e4" -> "e5" -> "Nf3"
// -------------------------------------------------------
struct TrieNode {
    string move;           // The move this node represents (e.g. "e4", "Nf3")
    int count;             // How many games from the database passed through here
    double avgScore;       // Average Stockfish eval at this position (optional, set later)

    // Each child key is a move string (e.g. "e5"),
    // and the value is a pointer to the next TrieNode
    unordered_map<string, TrieNode*> children;

    // Constructor — default move is empty string (used for root node)
    TrieNode(string m = "") : move(m), count(0), avgScore(0.0) {}
};


// -------------------------------------------------------
// ChessTrie
// Stores millions of chess games as a trie of moves.
// Lets you query: "given the moves played so far,
// what move was played most often in this position?"
// -------------------------------------------------------
class ChessTrie {
private:
    TrieNode* root;   // The root node (represents the empty board, before any moves)

public:
    // Constructor — creates the root node
    ChessTrie() : root(new TrieNode()) {}

    // Destructor — recursively delete all nodes to avoid memory leaks
    ~ChessTrie() {
        deleteNode(root);
    }

    // -------------------------------------------------------
    // insertGame
    // Takes a full game as a flat list of moves in order:
    //   {"e4", "e5", "Nf3", "Nc6", "Bb5"}
    // Walks down the trie, creating nodes as needed,
    // and increments count at each node along the path.
    // -------------------------------------------------------
    void insertGame(const vector<string>& moves) {
        TrieNode* curr = root;   // Start at the root

        for (const string& move : moves) {
            // If this move hasn't been seen from this position before,
            // create a new child node for it
            if (curr->children.find(move) == curr->children.end()) {
                curr->children[move] = new TrieNode(move);
            }

            // Step into the child node for this move
            curr = curr->children[move];

            // Increment count — one more game passed through this node
            curr->count++;
        }
    }

    // -------------------------------------------------------
    // getBestMove
    // Given a list of moves already played in the current game,
    // navigate to that position in the trie and return whichever
    // child move appears in the most games (highest count).
    //
    // Returns "" if the position isn't in the database at all.
    // -------------------------------------------------------
    string getBestMove(const vector<string>& playedMoves) {
        TrieNode* curr = root;

        // Walk down to the current position
        for (const string& move : playedMoves) {
            auto it = curr->children.find(move);

            // If any move in the sequence isn't in the trie,
            // this position is not in our database — give up
            if (it == curr->children.end()) return "";

            curr = it->second;
        }

        // We're now at the node for the current board position.
        // Find which child move was played in the most games.
        string bestMove = "";
        int maxCount = 0;

        for (auto& [move, node] : curr->children) {
            if (node->count > maxCount) {
                maxCount = node->count;
                bestMove = move;
            }
        }

        return bestMove;   // Will be "" if no children (end of all lines)
    }

    // -------------------------------------------------------
    // getTopMoves
    // Like getBestMove, but returns the top N moves sorted
    // by frequency. Useful for showing a ranked suggestion list.
    // Returns pairs of {move, count}.
    // -------------------------------------------------------
    vector<pair<string, int>> getTopMoves(const vector<string>& playedMoves, int n = 3) {
        TrieNode* curr = root;

        // Navigate to current position
        for (const string& move : playedMoves) {
            auto it = curr->children.find(move);
            if (it == curr->children.end()) return {};
            curr = it->second;
        }

        // Collect all children into a sortable list
        vector<pair<string, int>> candidates;
        for (auto& [move, node] : curr->children) {
            candidates.push_back({move, node->count});
        }

        // Sort descending by count
        sort(candidates.begin(), candidates.end(),
             [](const pair<string,int>& a, const pair<string,int>& b) {
                 return a.second > b.second;
             });

        // Return top N (or fewer if not enough candidates)
        if ((int)candidates.size() > n) candidates.resize(n);
        return candidates;
    }

    // -------------------------------------------------------
    // prune
    // Removes any node whose count is below the threshold.
    // Call this after bulk-inserting all games to trim the
    // trie down to statistically meaningful lines only.
    // -------------------------------------------------------
    void prune(int minCount = 5) {
        pruneNode(root, minCount);
    }

private:
    // Recursive helper to delete all nodes (used in destructor)
    void deleteNode(TrieNode* node) {
        if (!node) return;
        for (auto& [move, child] : node->children) {
            deleteNode(child);
        }
        delete node;
    }

    // Recursive helper for prune()
    // Removes children whose count is below minCount,
    // then recurses into remaining children
    void pruneNode(TrieNode* node, int minCount) {
        if (!node) return;

        // Collect keys to delete (can't erase while iterating)
        vector<string> toDelete;
        for (auto& [move, child] : node->children) {
            if (child->count < minCount) {
                toDelete.push_back(move);
            }
        }

        // Delete the low-count children
        for (const string& move : toDelete) {
            deleteNode(node->children[move]);
            node->children.erase(move);
        }

        // Recurse into the surviving children
        for (auto& [move, child] : node->children) {
            pruneNode(child, minCount);
        }
    }
};
