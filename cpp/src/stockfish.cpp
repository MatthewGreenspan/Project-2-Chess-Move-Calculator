/**
 * Stockfish UCI integration - get best move from FEN.
 * Fallback: Lichess Cloud Eval API when Stockfish unavailable.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>

static std::string findStockfish() {
  const char* paths[] = {"stockfish", "/opt/homebrew/bin/stockfish", "/usr/local/bin/stockfish"};
  for (const char* p : paths) {
    if (access(p, X_OK) == 0) return p;
  }
  return "";
}

// URL-encode FEN for Lichess API (spaces -> %20)
static std::string urlEncodeFen(const std::string& fen) {
  std::string out;
  for (char c : fen) {
    if (c == ' ') out += "%20";
    else out += c;
  }
  return out;
}

// Fallback: query Lichess Cloud Eval API (requires curl, works without Stockfish)
static std::string getBestMoveFromLichess(const std::string& fen) {
  std::string encoded = urlEncodeFen(fen);
  std::string cmd = "curl -s -m 5 \"https://lichess.org/api/cloud-eval?fen=" + encoded + "\" 2>/dev/null";
  FILE* f = popen(cmd.c_str(), "r");
  if (!f) return "";
  char buf[4096];
  std::string json;
  while (fgets(buf, sizeof(buf), f)) json += buf;
  pclose(f);
  size_t pv = json.find("\"moves\":\"");
  if (pv == std::string::npos) return "";
  pv += 9;
  size_t end = json.find("\"", pv);
  if (end == std::string::npos) return "";
  std::string moves = json.substr(pv, end - pv);
  size_t space = moves.find(' ');
  if (space != std::string::npos) moves = moves.substr(0, space);
  return moves;  // e.g. "e2e4"
}

std::string getBestMoveFromStockfish(const std::string& fen, int /* movetimeMs */) {
  std::string exe = findStockfish();
  if (exe.empty()) return getBestMoveFromLichess(fen);

  int toEngine[2], fromEngine[2];
  if (pipe(toEngine) != 0 || pipe(fromEngine) != 0) return getBestMoveFromLichess(fen);

  pid_t pid = fork();
  if (pid < 0) return getBestMoveFromLichess(fen);
  if (pid == 0) {
    close(toEngine[1]);
    close(fromEngine[0]);
    dup2(toEngine[0], STDIN_FILENO);
    dup2(fromEngine[1], STDOUT_FILENO);
    close(toEngine[0]);
    close(fromEngine[1]);
    execlp(exe.c_str(), "stockfish", nullptr);
    _exit(127);
  }
  close(toEngine[0]);
  close(fromEngine[1]);

  FILE* out = fdopen(fromEngine[0], "r");
  FILE* in = fdopen(toEngine[1], "w");
  if (!out || !in) {
    if (out) fclose(out);
    if (in) fclose(in);
    return getBestMoveFromLichess(fen);
  }

  char line[512];
  fprintf(in, "uci\n");
  fflush(in);
  while (fgets(line, sizeof(line), out) && strstr(line, "uciok") == nullptr) {}

  fprintf(in, "setoption name Threads value 2\n");
  fprintf(in, "ucinewgame\n");
  fprintf(in, "position fen %s\n", fen.c_str());
  fprintf(in, "go depth 18\n");
  fflush(in);

  std::string bestMove;
  int lineCount = 0;
  const int maxLines = 50000;
  while (fgets(line, sizeof(line), out) && ++lineCount < maxLines) {
    if (strncmp(line, "bestmove ", 9) == 0) {
      char* rest = line + 9;
      char* end = strchr(rest, ' ');
      if (end) *end = '\0';
      else rest[strcspn(rest, "\r\n")] = '\0';
      if (strcmp(rest, "(none)") != 0) bestMove = rest;
      break;
    }
  }

  fprintf(in, "quit\n");
  fflush(in);
  fclose(in);
  fclose(out);

  if (bestMove.empty()) return getBestMoveFromLichess(fen);
  return bestMove;
}
