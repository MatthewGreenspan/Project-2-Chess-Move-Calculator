#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <functional>
#include <iostream>

using namespace std;

// holds the bits we keep from one pgn game
struct GameData {
    int whiteElo = 0;
    int blackElo = 0;
    vector<string> moves;
};


// reads pgn games and sends them out one at a time
class PGNParser {
private:
    int minElo;
    int maxElo;
    int maxDepth;

    // removes pgn {...} comments before tokenizing
    string cleanMoveText(const string& raw) {
        string result;
        bool inComment = false;

        for (char c : raw) {
            if (c == '{') { inComment = true;  continue; }
            if (c == '}') { inComment = false; continue; }
            if (!inComment) result += c;
        }

        return result;
    }

    // turns raw move text into clean san tokens
    vector<string> parseMoves(const string& moveText) {
        vector<string> moves;
        istringstream ss(cleanMoveText(moveText));
        string token;

        while (ss >> token) {
            if (token.back() == '.' ||
                token.find("...") != string::npos) continue;

            if (token == "*"    || token == "1-0" ||
                token == "0-1"  || token == "1/2-1/2") continue;

            // removes ! ? + # off the end
            while (!token.empty() &&
                   string("!?+#").find(token.back()) != string::npos) {
                token.pop_back();
            }

            if (!token.empty()) {
                moves.push_back(token);

                // stops once we got enough opening moves
                if ((int)moves.size() >= maxDepth * 2) break;
            }
        }

        return moves;
    }

    // pulls the elo number out of a header line
    int parseElo(const string& line) {
        size_t q1 = line.find('"');
        size_t q2 = line.rfind('"');

        if (q1 == string::npos || q1 == q2) return 0;

        string val = line.substr(q1 + 1, q2 - q1 - 1);

        if (val == "?") return 0;

        try { return stoi(val); }
        catch (...) { return 0; }
    }

public:
    // sets the elo range and move depth we want
    PGNParser(int minElo = 1000, int maxElo = 1800, int maxDepth = 20)
        : minElo(minElo), maxElo(maxElo), maxDepth(maxDepth) {}

    // full parse pass over the pgn file
    void parse(const string& filepath, function<void(const GameData&)> onGame) {
        ifstream file(filepath);
        if (!file.is_open()) {
            cerr << "Error: could not open file: " << filepath << endl;
            return;
        }

        string line;
        GameData current;
        string moveBuffer;
        bool inMoves = false;
        int gamesProcessed = 0;

        // handles one finished game then resets temp state
        auto flush = [&]() {
            if (!moveBuffer.empty()) {
                current.moves = parseMoves(moveBuffer);

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

        while (getline(file, line)) {
            if (line.empty()) {
                if (inMoves) flush();
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

        // catches the last game if file ends right after it
        if (inMoves) flush();

        cout << "Done. Total games inserted: " << gamesProcessed << endl;
    }

    // same parse flow but can stop early
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
