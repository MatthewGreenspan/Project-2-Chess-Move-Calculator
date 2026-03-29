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