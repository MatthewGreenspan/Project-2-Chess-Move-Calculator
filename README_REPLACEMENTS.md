# README Replacements

This file contains replacement text for the two current README files:

- `README.md`
- `cpp/README.md`

You can copy each section into the matching file.

---

## Root `README.md`

```md
# Chess Move Calculator

Chess Move Calculator is a desktop chess app built in C++ with SDL2. You can set up positions by dragging pieces, explore legal moves, compare opening suggestions, and calculate a best move using either local Stockfish or Lichess cloud evaluation.

The project is centered around a native GUI in the `cpp/` folder. If you want full build instructions, platform notes, and engine setup details, start with [`cpp/README.md`](cpp/README.md).

## What it does

- Lets you drag pieces onto the board to build custom positions
- Highlights legal moves for the selected piece
- Supports free piece placement for setup and testing
- Suggests opening moves from local PGN data
- Calculates a best move with Stockfish or a Lichess fallback
- Shows move information in the sidebar, including hash and trie reccomendation and runtime

## Quick start

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

### Windows

Use CMake with `vcpkg` from the `cpp/` directory. Full instructions are in [`cpp/README.md`](cpp/README.md).

## Basic use

- Click a piece to show its legal moves
- Click a highlighted square to make the move
- Drag pieces freely to set up custom positions
- Drag from the top or bottom palette to add pieces
- Use **Calculate best move** to get an opening or engine suggestion

## Best move sources

The app checks moves in this order:

1. Position-based opening data from `lichess_games.pgn`
2. Opening-prefix suggestions from the trie and hash map
3. Stockfish
4. Lichess cloud eval if Stockfish is not available

For the best experience, install Stockfish locally.

## Project layout

```text
cpp/
├── src/        source files
├── include/    headers and bundled libraries
├── assets/     piece images
├── scripts/    helper scripts
└── README.md   detailed build and setup guide
```

## Credits

Piece images in `cpp/assets/white/` and `cpp/assets/black/` come from [Sashite](https://sashite.dev/assets/chess/) and are CC0 / public domain.
```

---

## `cpp/README.md`

```md
# Chess Move Calculator (C++)

This folder contains the C++ SDL2 version of Chess Move Calculator. The app is designed for fast position setup and move exploration: you can drag pieces onto the board, inspect legal moves, compare opening suggestions from local PGN data, and ask the engine for a best move.

## Features

- Native desktop GUI built with SDL2
- Click-to-preview legal moves
- Drag-and-drop board editing
- Extra piece palettes for position setup and promotions
- Opening suggestions from `lichess_games.pgn`
- Best-move calculation with Stockfish
- Lichess cloud fallback when Stockfish is unavailable

## Dependencies

Required:

- SDL2
- SDL2_ttf
- CMake 3.16 or newer

Optional:

- Stockfish
- `curl` for Lichess cloud fallback

## Build with CMake

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

### Windows

1. Install Visual Studio with the C++ workload.
2. Install CMake.
3. Install `vcpkg` and set `VCPKG_ROOT`.
4. From the `cpp/` folder, run:

```bat
%VCPKG_ROOT%\vcpkg.exe install --triplet x64-windows
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release
build\Release\chess-calc.exe
```

## Build with Make

On macOS or Linux, you can also build from `cpp/` with:

```bash
make
./chess-calc
```

## Running the app

The executable expects access to the `assets/` directory. The easiest option is to run it from `cpp/`, or to keep the copied `assets/` folder next to the built executable.

## Stockfish setup

Stockfish is recommended because it powers best-move calculation outside the opening book and improves evaluation output in the sidebar.

### Option 1: install it system-wide

- macOS: `brew install stockfish`
- Windows or Linux: install Stockfish and make sure it is available on `PATH`

### Option 2: download it into the project

From `cpp/`:

```bash
sh scripts/fetch_stockfish.sh
```

This places the engine in `third_party/stockfish/`, which the app checks before falling back to `PATH`.

If Stockfish is missing, the app can still use Lichess cloud evaluation when `curl` and network access are available.

## Opening data

The repository includes `lichess_games.pgn`, which is used to build the opening suggestions shown in the app.

The current code uses that PGN in a few ways:

- A position-based lookup keyed from FEN
- A move-prefix trie
- A prefix hash map
- A PGN scan for next-move suggestions

In practice, that means the app can suggest common opening moves before it falls back to engine analysis.

If you replace `lichess_games.pgn`, use a valid PGN file with SAN moves.

## Controls

- Click a piece to show legal moves
- Click a legal square to make a move
- Drag a board piece to move or remove it
- Drag from the piece palettes to add pieces
- Use the turn buttons to switch between White to move and Black to move
- Use the reset button to restore the starting position
- Use **Calculate best move** to generate a suggestion

## Development notes

### Generate `compile_commands.json`

From `cpp/`:

```bash
sh gen_compile_commands.sh
```

You can also run:

```bash
make compile_commands
```

### Display note

If the app has scaling problems on a HiDPI display, try setting:

```bash
CHESS_CALC_DISABLE_HIGHDPI=1
```

before running the program.
```
