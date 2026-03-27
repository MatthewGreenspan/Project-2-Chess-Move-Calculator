#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <functional>
#include <iostream>

using namespace std;

// -------------------------------------------------------
// GameData
// Plain struct that holds everything we care about
// extracting from a single PGN game record.
// -------------------------------------------------------
struct GameData {
    int whiteElo = 0;               // White player's Elo rating
    int blackElo = 0;               // Black player's Elo rating
    vector<string> moves;           // All moves in order: {"e4","e5","Nf3",...}
};


// -------------------------------------------------------
// PGNParser
// Reads a .pgn file (pre-decompressed from Lichess .pgn.zst)
// and emits one GameData per game via a callback function.
//
// Filters games by Elo range and caps move depth so we
// only store opening lines, not full 80-move endgames.
// -------------------------------------------------------
class PGNParser {
private:
    int minElo;    // Only keep games where BOTH players >= minElo
    int maxElo;    // Only keep games where BOTH players <= maxElo
    int maxDepth;  // Max number of HALF-MOVES (plies) to store per game
                   // e.g. maxDepth=20 stores the first 10 full moves

    // -------------------------------------------------------
    // cleanMoveText
    // Strips out PGN comments enclosed in { curly braces }.
    // e.g. "3. Bb5 { This is the Ruy Lopez }" -> "3. Bb5 "
    // Also handles nested-ish cases gracefully.
    // -------------------------------------------------------
    string cleanMoveText(const string& raw) {
        string result;
        bool inComment = false;

        for (char c : raw) {
            if (c == '{') { inComment = true;  continue; }   // comment starts
            if (c == '}') { inComment = false; continue; }   // comment ends
            if (!inComment) result += c;                      // keep everything else
        }

        return result;
    }

    // -------------------------------------------------------
    // parseMoves
    // Splits a cleaned move-text string into individual SAN tokens.
    // Input looks like:  "1. e4 e5 2. Nf3 Nc6 3. Bb5 *"
    // Output looks like: {"e4", "e5", "Nf3", "Nc6", "Bb5"}
    //
    // Things we need to skip or clean up:
    //   - Move numbers:   "1."  "2."  "10."
    //   - Black move nums: "3..."
    //   - Result tokens:  "*"  "1-0"  "0-1"  "1/2-1/2"
    //   - Annotations:    "!" "?" "!!" "??" "++" "#" (attached to moves)
    // -------------------------------------------------------
    vector<string> parseMoves(const string& moveText) {
        vector<string> moves;
        istringstream ss(cleanMoveText(moveText));
        string token;

        while (ss >> token) {
            // Skip move number tokens like "1." or "3..."
            // A token is a move number if it ends in '.' or contains "..."
            if (token.back() == '.' ||
                token.find("...") != string::npos) continue;

            // Skip result tokens — these mark the end of the game
            if (token == "*"    || token == "1-0" ||
                token == "0-1"  || token == "1/2-1/2") continue;

            // Strip trailing annotation characters from moves
            // e.g. "Nf3!" -> "Nf3",  "Bxe5??" -> "Bxe5",  "O-O+" -> "O-O"
            // We loop because there can be multiple: "Rxf7!+" has two chars to strip
            while (!token.empty() &&
                   string("!?+#").find(token.back()) != string::npos) {
                token.pop_back();
            }

            // After stripping, make sure we still have something
            if (!token.empty()) {
                moves.push_back(token);

                // Stop once we've hit our depth cap.
                // maxDepth is in full moves, so we multiply by 2 for half-moves (plies)
                if ((int)moves.size() >= maxDepth * 2) break;
            }
        }

        return moves;
    }

    // -------------------------------------------------------
    // parseElo
    // Extracts the integer Elo from a PGN header tag line.
    // Input:  [WhiteElo "1423"]
    // Output: 1423
    //
    // Returns 0 if the value is "?" (anonymous player)
    // or if parsing fails for any reason.
    // -------------------------------------------------------
    int parseElo(const string& line) {
        // Find the opening and closing quote marks
        size_t q1 = line.find('"');
        size_t q2 = line.rfind('"');

        // If we can't find two distinct quotes, something is malformed
        if (q1 == string::npos || q1 == q2) return 0;

        // Extract the string between the quotes, e.g. "1423"
        string val = line.substr(q1 + 1, q2 - q1 - 1);

        // "?" means the player is anonymous — treat as 0 (will be filtered out)
        if (val == "?") return 0;

        // Convert to int; catch any conversion errors gracefully
        try { return stoi(val); }
        catch (...) { return 0; }
    }

public:
    // -------------------------------------------------------
    // Constructor
    // minElo / maxElo: Elo range to accept (both players must qualify)
    // maxDepth: how many FULL moves to store (default 20 = 40 plies)
    // -------------------------------------------------------
    PGNParser(int minElo = 1000, int maxElo = 1800, int maxDepth = 20)
        : minElo(minElo), maxElo(maxElo), maxDepth(maxDepth) {}

    // -------------------------------------------------------
    // parse
    // Opens a .pgn file and reads it game by game.
    // For each valid game (Elo in range, has moves),
    // calls onGame(gameData) so the caller can insert it into the trie.
    //
    // Uses a callback (lambda) so we never load the whole file into memory —
    // we process and discard one game at a time.
    //
    // PGN format structure:
    //   [Header tags]     <- lines starting with '['
    //   <blank line>
    //   move text...      <- everything else
    //   <blank line>      <- signals end of game
    // -------------------------------------------------------
    void parse(const string& filepath, function<void(const GameData&)> onGame) {
        ifstream file(filepath);
        if (!file.is_open()) {
            cerr << "Error: could not open file: " << filepath << endl;
            return;
        }

        string line;
        GameData current;      // Accumulates data for the game we're currently reading
        string moveBuffer;     // Accumulates raw move text (may span multiple lines)
        bool inMoves = false;  // Are we currently past the header, in the move section?
        int gamesProcessed = 0;

        // -------------------------------------------------------
        // flush lambda
        // Called whenever we've finished reading a complete game.
        // Parses the accumulated move buffer, checks Elo filters,
        // and fires the callback if everything passes.
        // Then resets state for the next game.
        // -------------------------------------------------------
        auto flush = [&]() {
            if (!moveBuffer.empty()) {
                current.moves = parseMoves(moveBuffer);

                // Only emit this game if:
                //   1. Both players are within our Elo range
                //   2. We actually got some moves out of it
                bool eloOk = current.whiteElo >= minElo && current.whiteElo <= maxElo &&
                             current.blackElo >= minElo && current.blackElo <= maxElo;

                if (eloOk && !current.moves.empty()) {
                    onGame(current);
                    gamesProcessed++;
                }
            }

            // Reset for the next game
            current = GameData();
            moveBuffer.clear();
            inMoves = false;
        };

        // -------------------------------------------------------
        // Main read loop — process the file line by line
        // -------------------------------------------------------
        while (getline(file, line)) {

            // A blank line is our separator.
            // If we were collecting moves, this signals end of game.
            if (line.empty()) {
                if (inMoves) flush();
                continue;
            }

            if (line[0] == '[') {
                // -----------------------------------------------
                // Header tag line — extract fields we care about.
                // We only need WhiteElo and BlackElo for filtering.
                // Everything else (Event, Site, Date, etc.) we skip.
                // -----------------------------------------------
                if (line.find("[WhiteElo") != string::npos)
                    current.whiteElo = parseElo(line);
                else if (line.find("[BlackElo") != string::npos)
                    current.blackElo = parseElo(line);

                inMoves = false;   // Still in header section

            } else {
                // -----------------------------------------------
                // Move text line — append to our running buffer.
                // A single game's moves can span multiple lines in PGN,
                // so we keep accumulating until a blank line signals the end.
                // -----------------------------------------------
                inMoves = true;
                moveBuffer += " " + line;
            }
        }

        // The file might not end with a blank line — flush the last game manually
        if (inMoves) flush();

        cout << "Done. Total games inserted: " << gamesProcessed << endl;
    }

    /** Like parse(), but stops when onGame returns false (e.g. time / confidence budget). */
    bool parseWithEarlyExit(const string& filepath, function<bool(const GameData&)> onGame) {
        ifstream file(filepath);
        if (!file.is_open()) {
            cerr << "Error: could not open file: " << filepath << endl;
            return false;
        }

        string line;
        GameData current;
        string moveBuffer;
        bool inMoves = false;
        bool stop = false;

        auto flush = [&]() {
            if (!moveBuffer.empty()) {
                current.moves = parseMoves(moveBuffer);

                bool eloOk = current.whiteElo >= minElo && current.whiteElo <= maxElo &&
                             current.blackElo >= minElo && current.blackElo <= maxElo;

                if (eloOk && !current.moves.empty()) {
                    if (!onGame(current)) stop = true;
                }
            }

            current = GameData();
            moveBuffer.clear();
            inMoves = false;
        };

        while (getline(file, line) && !stop) {
            if (line.empty()) {
                if (inMoves) flush();
                if (stop) break;
                continue;
            }

            if (line[0] == '[') {
                if (line.find("[WhiteElo") != string::npos)
                    current.whiteElo = parseElo(line);
                else if (line.find("[BlackElo") != string::npos)
                    current.blackElo = parseElo(line);
                inMoves = false;
            } else {
                inMoves = true;
                moveBuffer += " " + line;
            }
        }

        if (inMoves && !stop) flush();
        return !stop;
    }
};
