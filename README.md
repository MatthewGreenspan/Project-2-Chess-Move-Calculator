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