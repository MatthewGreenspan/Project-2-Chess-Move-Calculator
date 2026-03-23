# Chess Move Calculator

Native C++ chess puzzle setup app — drag pieces to set up positions, use palettes for promotions. Opens as a window on your desktop.

## Build (macOS)

```bash
# Install dependencies (one time)
brew install sdl2
brew install sdl2_ttf   # Optional, for sidebar text

# Build
cd cpp
make
```

## Run

```bash
cd cpp
./chess-calc
```

A native window will pop up with the chess board — no browser needed.

## Usage

- **Drag pieces** on the board to move them anywhere (free placement) or to legal squares (makes a proper move)
- **Click a piece** to see its legal moves (highlighted in green)
- **Click a legal square** to make the move
- **Drag from palettes** (above/below board) to add pieces — for promotions or setup
- **Drag pieces onto palettes** to remove them from the board
- **Reset** — clear and start over
- **White to move** / **Black to move** — set whose turn and flip the board view
- **Sidebar** — shows turn, legal moves list, and FEN (with sdl2_ttf installed)

## Project structure

```
cpp/
├── src/main.cpp       # GUI app (SDL2)
├── include/chess.hpp  # Chess logic library
├── assets/
│   ├── white/         # Piece PNGs (king, queen, rook, bishop, knight, pawn)
│   └── black/
└── CMakeLists.txt
```

## Dependencies

- **SDL2** — window, rendering, input (`brew install sdl2`)
- **SDL2_ttf** — optional, for sidebar text (`brew install sdl2_ttf`)
- **stb_image** — PNG loading (bundled in `include/stb_image.h`)
- **chess-library** — move generation, FEN (bundled in `include/chess.hpp`)

## Assets

Chess piece PNGs in `cpp/assets/white/` and `cpp/assets/black/` from [Sashité](https://sashite.dev/assets/chess/), CC0 (public domain). Gray placeholders used if PNGs are missing.
