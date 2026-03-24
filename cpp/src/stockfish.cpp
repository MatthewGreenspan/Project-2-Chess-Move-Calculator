/**
 * Stockfish UCI integration - get best move from FEN.
 * Fallback: Lichess Cloud Eval API when Stockfish unavailable.
 * Unix: fork + pipes. Windows: CreateProcess + pipes.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

static std::string urlEncodeFen(const std::string& fen) {
  std::string out;
  for (char c : fen) {
    if (c == ' ') out += "%20";
    else out += c;
  }
  return out;
}

static std::string getBestMoveFromLichess(const std::string& fen) {
  std::string encoded = urlEncodeFen(fen);
#ifdef _WIN32
  std::string cmd =
      "curl.exe -s -m 5 \"https://lichess.org/api/cloud-eval?fen=" + encoded + "\" 2>nul";
  FILE* f = _popen(cmd.c_str(), "r");
#else
  std::string cmd =
      "curl -s -m 5 \"https://lichess.org/api/cloud-eval?fen=" + encoded + "\" 2>/dev/null";
  FILE* f = popen(cmd.c_str(), "r");
#endif
  if (!f) return "";
  char buf[4096];
  std::string json;
  while (fgets(buf, sizeof(buf), f)) json += buf;
#ifdef _WIN32
  _pclose(f);
#else
  pclose(f);
#endif
  size_t pv = json.find("\"moves\":\"");
  if (pv == std::string::npos) return "";
  pv += 9;
  size_t end = json.find("\"", pv);
  if (end == std::string::npos) return "";
  std::string moves = json.substr(pv, end - pv);
  size_t space = moves.find(' ');
  if (space != std::string::npos) moves = moves.substr(0, space);
  return moves;
}

#ifdef _WIN32

static bool fileExists(const char* p) {
  DWORD a = GetFileAttributesA(p);
  return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

static std::string findStockfish() {
  char buf[MAX_PATH];
  if (SearchPathA(nullptr, "stockfish.exe", nullptr, sizeof(buf), buf, nullptr) > 0) return std::string(buf);
  if (SearchPathA(nullptr, "stockfish", ".exe", sizeof(buf), buf, nullptr) > 0) return std::string(buf);
  const char* fixed[] = {
      "C:\\Program Files\\Stockfish\\stockfish-windows-x86-64-avx2.exe",
      "C:\\Program Files\\Stockfish\\stockfish.exe",
  };
  for (const char* p : fixed) {
    if (fileExists(p)) return p;
  }
  return "";
}

static bool writeAll(HANDLE h, const char* data, size_t n) {
  size_t off = 0;
  while (off < n) {
    DWORD w = 0;
    if (!WriteFile(h, data + off, (DWORD)(n - off), &w, nullptr) || w == 0) return false;
    off += w;
  }
  return true;
}

static bool readLine(HANDLE h, std::string& line) {
  line.clear();
  char c;
  DWORD r = 0;
  for (;;) {
    if (!ReadFile(h, &c, 1, &r, nullptr) || r == 0) return !line.empty();
    if (c == '\n') break;
    if (c != '\r') line += c;
  }
  return true;
}

static std::string runStockfishUci(const std::string& exe, const std::string& fen) {
  SECURITY_ATTRIBUTES sa{};
  sa.nLength = sizeof(sa);
  sa.bInheritHandle = TRUE;

  HANDLE outRd = nullptr, outWr = nullptr;
  HANDLE inRd = nullptr, inWr = nullptr;
  if (!CreatePipe(&outRd, &outWr, &sa, 0)) return "";
  if (!CreatePipe(&inRd, &inWr, &sa, 0)) {
    CloseHandle(outRd);
    CloseHandle(outWr);
    return "";
  }
  SetHandleInformation(outRd, HANDLE_FLAG_INHERIT, 0);
  SetHandleInformation(inWr, HANDLE_FLAG_INHERIT, 0);

  STARTUPINFOA si{};
  si.cb = sizeof(si);
  si.hStdError = outWr;
  si.hStdOutput = outWr;
  si.hStdInput = inRd;
  si.dwFlags |= STARTF_USESTDHANDLES;

  PROCESS_INFORMATION pi{};
  BOOL ok = CreateProcessA(exe.c_str(), nullptr, nullptr, nullptr, TRUE,
                           CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
  CloseHandle(inRd);
  CloseHandle(outWr);
  if (!ok) {
    CloseHandle(outRd);
    CloseHandle(inWr);
    return "";
  }

  auto send = [&](const char* s) { return writeAll(inWr, s, strlen(s)); };
  std::string line;
  if (!send("uci\n")) {
    TerminateProcess(pi.hProcess, 1);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(outRd);
    CloseHandle(inWr);
    return "";
  }
  while (readLine(outRd, line)) {
    if (line.find("uciok") != std::string::npos) break;
    if (line.size() > 10000) break;
  }
  if (!send("isready\n")) {
    TerminateProcess(pi.hProcess, 1);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(outRd);
    CloseHandle(inWr);
    return "";
  }
  while (readLine(outRd, line)) {
    if (line.find("readyok") != std::string::npos) break;
    if (line.size() > 10000) break;
  }

  if (!send("setoption name Threads value 2\n") || !send("ucinewgame\n")) {
    TerminateProcess(pi.hProcess, 1);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(outRd);
    CloseHandle(inWr);
    return "";
  }
  std::string pos = "position fen " + fen + "\n";
  if (!send(pos.c_str()) || !send("go depth 18\n")) {
    TerminateProcess(pi.hProcess, 1);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(outRd);
    CloseHandle(inWr);
    return "";
  }

  std::string bestMove;
  int lineCount = 0;
  const int maxLines = 50000;
  while (readLine(outRd, line) && ++lineCount < maxLines) {
    if (line.size() >= 9 && line.compare(0, 9, "bestmove ") == 0) {
      std::string rest = line.substr(9);
      size_t sp = rest.find(' ');
      if (sp != std::string::npos) rest = rest.substr(0, sp);
      if (rest != "(none)") bestMove = rest;
      break;
    }
  }

  send("quit\n");
  CloseHandle(inWr);
  CloseHandle(outRd);
  WaitForSingleObject(pi.hProcess, 8000);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  return bestMove;
}

std::string getBestMoveFromStockfish(const std::string& fen, int) {
  std::string exe = findStockfish();
  if (exe.empty()) return getBestMoveFromLichess(fen);
  std::string uci = runStockfishUci(exe, fen);
  if (uci.empty()) return getBestMoveFromLichess(fen);
  return uci;
}

#else

static std::string findStockfish() {
  const char* paths[] = {"stockfish", "/opt/homebrew/bin/stockfish", "/usr/local/bin/stockfish"};
  for (const char* p : paths) {
    if (access(p, X_OK) == 0) return p;
  }
  return "";
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

  fprintf(in, "isready\n");
  fflush(in);
  while (fgets(line, sizeof(line), out) && strstr(line, "readyok") == nullptr) {}

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

#endif
