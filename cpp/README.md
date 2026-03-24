# Chess Move Calculator (C++)

Native window app for chess puzzle setup. Drag pieces, use palettes, view best move and possible moves in the sidebar.

## Platforms

| Platform | Status |
|----------|--------|
| **macOS** | Fully supported: `make` in `cpp/` (see below). |
| **Linux** | Should work: install SDL2, SDL2_ttf, Stockfish; adapt `Makefile` paths if needed. |
| **Windows** | The UI (`main.cpp`) is portable SDL2. **`stockfish.cpp` uses POSIX `fork()` / pipes**, which **do not compile with MSVC**. Teammates on Windows can: (1) use **WSL2** and build like Linux, (2) use **MSYS2** with a POSIX toolchain if available, or (3) temporarily exclude `stockfish.cpp` and rely on the Lichess HTTP fallback only—**a native Windows Stockfish subprocess still needs a small port** (e.g. `CreateProcess` + pipes). |

The repo `.gitignore` is set up so Mac and Windows build junk and IDE files are not committed.

## Build (macOS)

```bash
brew install sdl2 sdl2_ttf stockfish
cd cpp
make
```

**Important:** `sdl2_ttf` is required for the sidebar text. Without it, the sidebar will be empty.

## Run

```bash
./chess-calc
```

Stockfish is used for best-move analysis when you click **Calculate best move**. If Stockfish is missing, the app falls back to the Lichess Cloud API (needs `curl` and network). If that fails too, the first legal move is shown.
