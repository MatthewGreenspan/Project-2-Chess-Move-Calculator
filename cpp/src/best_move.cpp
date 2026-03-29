#include "best_move.hpp"
#include "chess_gui_helpers.hpp"
#include "opening_db.hpp"
#include "opening_scan.hpp"
#include "opening_suggestion.hpp"
#include "stockfish.hpp"
#include "fen_parser.hpp"

#include <chrono>
#include <cstdio>
#include <string>
#include <vector>

using namespace std;
using namespace chess;
using namespace chess_gui;

namespace {

constexpr int kOpeningPliesSkipKingWalk = 28;
constexpr int kMaxOpeningTriePlies = 40;
constexpr double kScanTimeLimitMs = 5000.0;
constexpr double kScanConfidence = 0.90;
constexpr int kScanMinGames = 5;

// formats cp or mate into the sidebar grade text
static string formatEvalGradeLabel(int cp, bool mate, int mateIn, const char* src) {
  if (mate) {
    if (mateIn > 0)
      return string(src) + ": they mate in " + to_string(mateIn) + " (bad for last move)";
    return string(src) + ": you mate in " + to_string(-mateIn) + " (good for last move)";
  }
  int moverCp = -cp;
  char buf[144];
  snprintf(buf, sizeof(buf), "%s: %+0.2f pawns (last mover ~cp %d)", src, moverCp / 100.0, moverCp);
  return string(buf);
}

// checks how good one move was after its played
static string formatMoveGrade(const Board& rootBoard, Move m) {
  Board b = rootBoard;
  b.makeMove(m);
  string fenAfter = b.getFen();
  int cp = 0;
  bool mate = false;
  int mateIn = 0;
  if (stockfishEvalPosition(fenAfter, cp, mate, mateIn))
    return formatEvalGradeLabel(cp, mate, mateIn, "SF d14");
  if (lichessCloudEvalPosition(fenAfter, cp, mate, mateIn))
    return formatEvalGradeLabel(cp, mate, mateIn, "Lichess");
  return "Grade: add Stockfish (sh scripts/fetch_stockfish.sh or brew) or use network";
}

/** 0–100 from the mover's perspective (higher = better for the side that just played). */
static int scorePercentForMove(const Board& rootBoard, Move m) {
  Board b = rootBoard;
  b.makeMove(m);
  string fenAfter = b.getFen();
  int cp = 0;
  bool mate = false;
  int mateIn = 0;
  if (!stockfishEvalPosition(fenAfter, cp, mate, mateIn)) {
    if (!lichessCloudEvalPosition(fenAfter, cp, mate, mateIn)) return -1;
  }
  if (mate) {
    if (mateIn > 0) return 0;
    return 100;
  }
  int moverCp = -cp;
  int p = 50 + moverCp / 25;
  if (p < 0) p = 0;
  if (p > 100) p = 100;
  return p;
}

static Move findFirstLegalFromRanked(const Board& board, const Movelist& moves,
                                     const vector<pair<string, int>>& ranked, bool inOpening) {
  for (const auto& entry : ranked) {
    const string& san = entry.first;
    if (san.empty()) continue;
    if (inOpening && sanIsNonCastlingKingMove(san)) continue;
    for (size_t i = 0; i < moves.size(); i++) {
      if (uci::moveToSan(board, moves[i]) == san) return moves[i];
    }
  }
  return Move::NO_MOVE;
}

// stores the move arrow squares into app state
static void setArrow(App& app, Move m, bool primary) {
  if (primary) {
    app.bestMoveFrom = m.from().index();
    app.bestMoveTo = m.to().index();
    app.showBestMoveArrow = true;
  } else {
    app.secondBestMoveFrom = m.from().index();
    app.secondBestMoveTo = m.to().index();
    app.showSecondBestArrow = true;
  }
}

}  // namespace

// clears all the move suggestion text/arrows
void clearBestMoveDisplay(App& app) {
  app.bestMoveFrom = -1;
  app.bestMoveTo = -1;
  app.secondBestMoveFrom = -1;
  app.secondBestMoveTo = -1;
  app.showBestMoveArrow = false;
  app.showSecondBestArrow = false;
  app.openingLookupCompare.clear();
  app.openingTrieLine.clear();
  app.openingTrieSecondLine.clear();
  app.openingHashLine.clear();
  app.openingHashSecondLine.clear();
  app.openingScanLine.clear();
  app.moveGradeLine.clear();
}

// main best move pipeline for book/db/engine lookup
void updateBestMove(App& app, bool force) {
  string fen = app.board.getFen();
  if (!force && fen == app.lastAnalyzedFen) return;
  app.lastAnalyzedFen = fen;
  app.bestMoveUci = "";
  app.bestMoveSan = "";
  app.bestMoveEnglish = "";
  app.openingLookupCompare.clear();
  app.trieTimingLine.clear();
  app.hashTimingLine.clear();
  app.prefixGamesLine.clear();
  app.openingTrieLine.clear();
  app.openingTrieSecondLine.clear();
  app.openingHashLine.clear();
  app.openingHashSecondLine.clear();
  app.openingScanLine.clear();
  app.moveGradeLine.clear();
  app.bestMoveFrom = -1;
  app.bestMoveTo = -1;
  app.secondBestMoveFrom = -1;
  app.secondBestMoveTo = -1;
  app.showBestMoveArrow = false;
  app.showSecondBestArrow = false;

  if (!hasValidKingCount(app.board)) {
    app.bestMoveEnglish = "Invalid position (need 1 king each)";
    return;
  }
  Movelist moves;
  movegen::legalmoves(moves, app.board);
  if (moves.size() == 0) {
    app.bestMoveEnglish = "Game over";
    return;
  }

  bool hasPositionDbMove = false;
  {
    // first try the direct fen based opening db
    string bookMove = lookupPositionDBMove(fen);
    if (!bookMove.empty()) {
      for (size_t i = 0; i < moves.size(); i++) {
        if (uci::moveToSan(app.board, moves[i]) == bookMove) {
          app.bestMoveSan = bookMove;
          app.bestMoveEnglish = "Book (position DB)";
          setArrow(app, moves[i], true);
          app.moveGradeLine = formatMoveGrade(app.board, moves[i]);
          hasPositionDbMove = true;
          break;
        }
      }
    }
  }

  if (static_cast<int>(app.movesPlayed.size()) < kMaxOpeningTriePlies) {
    using clock = chrono::high_resolution_clock;
    auto t0 = clock::now();
    // gets ranked opening ideas from both structures
    vector<pair<string, int>> rankedTrie = g_trie.getRankedMoves(app.movesPlayed, 16);
    auto t1 = clock::now();
    auto t2 = clock::now();
    vector<pair<string, int>> rankedHash = g_openingHash.getRankedMoves(app.movesPlayed, 16);
    auto t3 = clock::now();
    long long nsTrie = chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();
    long long nsHash = chrono::duration_cast<chrono::nanoseconds>(t3 - t2).count();
    double msTrie = nsTrie / 1e6;
    double msHash = nsHash / 1e6;
    char timingBuf[128];
    snprintf(timingBuf, sizeof(timingBuf), "Trie time: %.3f ms (%lld ns)", msTrie, (long long)nsTrie);
    app.trieTimingLine = timingBuf;
    snprintf(timingBuf, sizeof(timingBuf), "Hash time: %.3f ms (%lld ns)", msHash, (long long)nsHash);
    app.hashTimingLine = timingBuf;


    // sums how many games matched this prefix
    int triePrefixGames = 0;
    for (const auto& entry : rankedTrie) triePrefixGames += entry.second;
    int hashPrefixGames = 0;
    for (const auto& entry : rankedHash) hashPrefixGames += entry.second;
    snprintf(timingBuf, sizeof(timingBuf), "Prefix games: %d", max(triePrefixGames, hashPrefixGames));
    app.prefixGamesLine = timingBuf;
    
    const bool inOpening = static_cast<int>(app.movesPlayed.size()) < kOpeningPliesSkipKingWalk;

    Move mt1 = findFirstLegalFromRanked(app.board, moves, rankedTrie, inOpening);
    Move mt2 = Move::NO_MOVE;
    if (rankedTrie.size() >= 2) {
      vector<pair<string, int>> sub(rankedTrie.begin() + 1, rankedTrie.end());
      mt2 = findFirstLegalFromRanked(app.board, moves, sub, inOpening);
    }

    Move mh1 = findFirstLegalFromRanked(app.board, moves, rankedHash, inOpening);
    Move mh2 = Move::NO_MOVE;
    if (rankedHash.size() >= 2) {
      vector<pair<string, int>> sub(rankedHash.begin() + 1, rankedHash.end());
      mh2 = findFirstLegalFromRanked(app.board, moves, sub, inOpening);
    }

    // builds the sidebar text for top 1 and top 2 moves
    auto buildTwoLines = [&](const char* label, Move m1, Move m2, string& line1, string& line2) {
      char buf[320];
      if (m1 == Move::NO_MOVE) {
        snprintf(buf, sizeof(buf), "%s #1: no legal book move", label);
        line1 = buf;
        line2.clear();
        return;
      }
      int s1 = scorePercentForMove(app.board, m1);
      if (s1 < 0)
        snprintf(buf, sizeof(buf), "%s #1: %s", label, uci::moveToSan(app.board, m1).c_str());
      else
        snprintf(buf, sizeof(buf), "%s #1: %s score %d/100", label, uci::moveToSan(app.board, m1).c_str(), s1);
      line1 = buf;
      if (m2 != Move::NO_MOVE && m2 != m1) {
        int s2 = scorePercentForMove(app.board, m2);
        if (s2 < 0)
          snprintf(buf, sizeof(buf), "%s #2: %s", label, uci::moveToSan(app.board, m2).c_str());
        else
          snprintf(buf, sizeof(buf), "%s #2: %s score %d/100", label, uci::moveToSan(app.board, m2).c_str(), s2);
        line2 = buf;
      } else {
        line2.clear();
      }
    };

    if (rankedTrie.empty()) {
      app.openingTrieLine = "Trie #1: no prefix in DB";
      app.openingTrieSecondLine.clear();
    } else
      buildTwoLines("Trie", mt1, mt2, app.openingTrieLine, app.openingTrieSecondLine);

    if (rankedHash.empty()) {
      app.openingHashLine = "Hash #1: no prefix in DB";
      app.openingHashSecondLine.clear();
    } else
      buildTwoLines("Hash", mh1, mh2, app.openingHashLine, app.openingHashSecondLine);

    // pgn scan is slower but helps if trie/hash dont have a clean answer
    PgnScanResult scan = scanPgnForNextMoves(getOpeningPgnPath(), app.movesPlayed, kScanTimeLimitMs, kScanConfidence,
                                             kScanMinGames);
    if (scan.fileMissing) {
      app.openingScanLine = "PGN scan: (no lichess_games.pgn)";
    } else {
      char sbuf[280];
      const char* why = scan.stoppedByConfidence ? "90% confidence" : (scan.stoppedByTime ? "time limit" : "EOF");
      snprintf(sbuf, sizeof(sbuf), "PGN scan: %.0f ms, %d games w/ next move / %d seen — %s", scan.elapsedMs,
                    scan.gamesWithNextMove, scan.gamesVisited, why);
      app.openingScanLine = sbuf;
    }

    Move mScan1 = Move::NO_MOVE;
    Move mScan2 = Move::NO_MOVE;
    if (!scan.rankedNext.empty()) mScan1 = findFirstLegalFromRanked(app.board, moves, scan.rankedNext, inOpening);
    if (scan.rankedNext.size() >= 2) {
      vector<pair<string, int>> sub(scan.rankedNext.begin() + 1, scan.rankedNext.end());
      mScan2 = findFirstLegalFromRanked(app.board, moves, sub, inOpening);
    }

    enum class Src { None, Scan, Trie, Hash };
    Src src = Src::None;
    Move primary = Move::NO_MOVE;
    Move secondary = Move::NO_MOVE;

    // picks the first source that gave us a legal book move
    if (mt1 != Move::NO_MOVE) {
      primary = mt1;
      secondary = mt2;
      src = Src::Trie;
    } else if (mh1 != Move::NO_MOVE) {
      primary = mh1;
      secondary = mh2;
      src = Src::Hash;
    } else if (mScan1 != Move::NO_MOVE) {
      primary = mScan1;
      secondary = mScan2;
      src = Src::Scan;
    }

    if (!hasPositionDbMove && primary != Move::NO_MOVE) {
      app.bestMoveSan = uci::moveToSan(app.board, primary);
      if (src == Src::Trie)
        app.bestMoveEnglish = "Book (trie — green arrow)";
      else if (src == Src::Hash)
        app.bestMoveEnglish = "Book (hash map — green arrow)";
      else
        app.bestMoveEnglish = "Book (PGN scan — green arrow)";
      setArrow(app, primary, true);
      if (secondary != Move::NO_MOVE && secondary != primary) setArrow(app, secondary, false);
      app.moveGradeLine = formatMoveGrade(app.board, primary);
      return;
    }
  }

  // if book lookup already solved it were done
  if (hasPositionDbMove) return;

  // final fallback is engine or lichess cloud
  string uci = getBestMoveFromStockfish(fen, 400);
  Move m = Move::NO_MOVE;
  if (!uci.empty()) m = uci::uciToMove(app.board, uci);
  if (m == Move::NO_MOVE) m = moves[0];

  app.bestMoveUci = uci;
  app.bestMoveSan = uci::moveToSan(app.board, m);
  app.bestMoveEnglish = moveToPlainEnglish(app.board, m);
  setArrow(app, m, true);
  app.moveGradeLine = formatMoveGrade(app.board, m);
}
