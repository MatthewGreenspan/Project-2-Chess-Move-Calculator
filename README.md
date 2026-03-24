# Chess Move Calculator

Native C++ chess puzzle setup app — drag pieces to set up positions, use palettes for promotions. Opens as a window on your desktop.

## Build

**Full instructions (Windows, macOS, Linux, vcpkg, CMake):** **[cpp/README.md](cpp/README.md)**

### macOS / Linux (short)

```bash
brew install sdl2 sdl2_ttf pkg-config cmake   # macOS; use apt on Debian/Ubuntu — see cpp/README.md
cd cpp && cmake -B build -S . && cmake --build build && ./build/chess-calc
```

### Windows (short)

Install **Visual Studio** (C++ workload), **CMake**, and **[vcpkg](https://github.com/microsoft/vcpkg)**. Set **`VCPKG_ROOT`**, then from **`cpp/`**:

```bat
%VCPKG_ROOT%\vcpkg.exe install --triplet x64-windows
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release
build\Release\chess-calc.exe
```

Details, Stockfish, and `curl` are in **[cpp/README.md](cpp/README.md)**. The repo includes **`cpp/vcpkg.json`** so SDL2 versions match across machines.

**Makefile** (`make` in `cpp/`) works on **macOS/Linux only** if you prefer not to use CMake there.

## Run

Use the `chess-calc` binary next to the `assets/` folder (CMake copies assets on build; with `make`, run from `cpp/`).

## Usage

- **Drag pieces** on the board to move them anywhere (free placement) or to legal squares (makes a proper move)
- **Click a piece** to see its legal moves (highlighted in green)
- **Click a legal square** to make the move
- **Drag from palettes** (above/below board) to add pieces — for promotions or setup
- **Drag pieces onto palettes** to remove them from the board
- **Reset** — clear and start over
- **White to move** / **Black to move** — set whose turn and flip the board view
- **Sidebar** — turn, best move, possible moves, FEN (needs SDL2_ttf)

## Project structure

```
cpp/
├── src/main.cpp       # GUI app (SDL2)
├── src/stockfish.cpp  # Engine + Lichess fallback
├── include/chess.hpp  # Chess logic library
├── vcpkg.json         # Pinned SDL2 deps (Windows / vcpkg)
├── assets/
│   ├── white/         # Piece PNGs
│   └── black/
└── CMakeLists.txt
```

## Dependencies

- **SDL2** — window, rendering, input  
- **SDL2_ttf** — sidebar text  
- **stb_image** — PNG loading (bundled in `include/stb_image.h`)  
- **chess-library** — move generation, FEN (bundled in `include/chess.hpp`)  

## Assets

Chess piece PNGs in `cpp/assets/white/` and `cpp/assets/black/` from [Sashité](https://sashite.dev/assets/chess/), CC0 (public domain).
