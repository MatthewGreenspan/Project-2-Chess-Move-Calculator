# Chess Move Calculator (C++)

Native SDL2 app: puzzle setup, palettes, opening statistics (trie + hash map), Stockfish best move, optional SF-grade line in the sidebar.

## Dependencies (all platforms)

| Package | Purpose |
|--------|---------|
| **SDL2** | Window, rendering, input |
| **SDL2_ttf** | Sidebar text (without it, the sidebar is blank) |
| **CMake** 3.16+ | Build |

Optional: **curl** (for Lichess cloud-eval fallback if Stockfish is not installed).

---

## Build (CMake) — recommended

### macOS

```bash
brew install sdl2 sdl2_ttf pkg-config cmake
cd cpp
cmake -B build -S .
cmake --build build
./build/chess-calc
```

### Linux (Debian / Ubuntu)

```bash
sudo apt install libsdl2-dev libsdl2-ttf-dev pkg-config cmake build-essential
cd cpp
cmake -B build -S .
cmake --build build
./build/chess-calc
```

### Run directory

The executable loads **`assets/`** using paths relative to the **current working directory**. Run from **`cpp/`** as above, or copy `assets/` next to the binary if you move it.

### Without pkg-config (macOS / Linux)

```bash
cmake -B build -S . -DCMAKE_PREFIX_PATH=/path/to/sdl2/prefix
cmake --build build
```

---

## Build (Makefile) — macOS / Linux only

From **`cpp/`**:

```bash
make
./chess-calc
```

Produces **`chess-calc`** in **`cpp/`** (not under `build/`). Requires SDL2 and SDL2_ttf via Homebrew or `pkg-config` (see Makefile comments).

---

## Windows build (step by step)

### 1. Install tools

- **[Visual Studio 2022](https://visualstudio.microsoft.com/)** (or **Build Tools**) with workload **“Desktop development with C++”** (MSVC, Windows SDK).
- **[CMake](https://cmake.org/download/)** — add to PATH, or use the VS-bundled CMake.
- **[Git for Windows](https://git-scm.com/download/win)** — provides **`curl.exe`** (Lichess fallback when Stockfish is missing).

### 2. Install vcpkg (once per machine)

```powershell
cd C:\dev
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

Set **`VCPKG_ROOT`** (e.g. `C:\dev\vcpkg`).

### 3. Configure and build

From the repo’s **`cpp`** folder (where **`vcpkg.json`** lives):

```bat
cd path\to\Project-2-Chess-Move-Calculator\cpp
%VCPKG_ROOT%\vcpkg.exe install --triplet x64-windows
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release
```

Run (from **`cpp`**):

```bat
build\Release\chess-calc.exe
```

CMake copies **`assets`** and vcpkg **runtime DLLs** next to **`chess-calc.exe`**. Run the `.exe` from **`build\Release\`** (or keep those DLLs beside the exe if you move it).

---

## Stockfish (local engine)

The app uses **Stockfish** for:

- **Best move** (when not using the opening book, or after the book window)
- **Sidebar grade** (depth-14 eval after the suggested move) — **local binary only**

### Option A — package manager

- **macOS:** `brew install stockfish`
- **Windows / Linux:** install from [stockfishchess.org](https://stockfishchess.org/download/) and ensure `stockfish` / `stockfish.exe` is on **PATH** (or place the Windows binary under `C:\Program Files\Stockfish\` as documented in the code).

### Option B — project-local binary (any OS)

From **`cpp/`**:

```bash
python3 scripts/fetch_stockfish.py
```

This downloads the latest official release into **`third_party/stockfish/`** (gitignored). The app checks **`third_party/stockfish/stockfish`** (or **`stockfish.exe`**) before **PATH**.

### If Stockfish is missing

- **Best move** falls back to **Lichess cloud eval** (needs **`curl`** and network).
- **Grade** shows *`SF grade: (need local Stockfish)`* — no cloud eval for grading.

---

## Opening database (`lichess_games.pgn`)

The repo includes **`lichess_games.pgn`** with many **full** games (random legal play, Elo headers **1800–2500**). A **FEN-keyed position hash map** (first 20 plies per game) suggests **Book (position DB)** moves when the current board matches training data. A **move-prefix trie** and a second **prefix hash map** back “most popular” lines; the sidebar can show **Trie vs Hash** lookup timing (nanoseconds) in the opening window.

Regenerate (requires **python-chess**):

```bash
cd cpp/scripts && pip install -r requirements.txt
python3 build_sample_pgn.py --out ../lichess_games.pgn
```

If your shell is already in **`cpp/`**:

```bash
cd scripts && pip install -r requirements.txt
python3 build_sample_pgn.py --out ../lichess_games.pgn
```

**Fetch real rated games** (Lichess HTTP API):

```bash
cd cpp/scripts && python3 fetch_lichess_sample_pgn.py --out ../lichess_games.pgn --min 1800 --max 2500 \
  --users SomeClubPlayer --users AnotherPlayer --max-games 80
```

**Bulk data:** [Lichess database](https://database.lichess.org/).

The app loads games with **both** players rated **1800–2500** and stores up to **40 full moves** per game in the trie/hash map (`PGNParser` in `src/opening_db.cpp`). The book is used only for the **first 20 full moves** (40 half-moves) of the move list; after that, **Stockfish** (or Lichess) is used. In the first **14 full moves**, non-castling **king** suggestions from the DB are skipped.

Editing the board, removing pieces, or switching **To move** clears the move list so the book cannot apply to a mismatched position.

---

## Shell tip

Do not start a command line with **`&&`** (e.g. `&& ./chess-calc`); use **`./chess-calc`** alone or chain after a real command: **`make && ./chess-calc`**.

---

## Display (HiDPI / Retina)

If the board looks wrong after a full rebuild:

1. `make clean && make` or a clean CMake build.
2. **macOS / Retina:** `CHESS_CALC_DISABLE_HIGHDPI=1 ./chess-calc`
3. **Windows:** set the same environment variable before running `chess-calc.exe`.
