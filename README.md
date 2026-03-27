# Chess Move Calculator

Native C++ chess puzzle app — drag pieces to set up positions, palettes for promotions, opening statistics, and engine-backed best move. Opens as a desktop window.

**Full build and run instructions:** **[cpp/README.md](cpp/README.md)**

## Quick build (macOS / Linux)

```bash
brew install sdl2 sdl2_ttf pkg-config cmake   # macOS; Debian/Ubuntu: see cpp/README.md
cd cpp && cmake -B build -S . && cmake --build build && ./build/chess-calc
```

Or with **Make** (Unix, from `cpp/`):

```bash
cd cpp && make && ./chess-calc
```

## Quick build (Windows)

Install **Visual Studio** (C++ workload), **CMake**, and **[vcpkg](https://github.com/microsoft/vcpkg)**. Set **`VCPKG_ROOT`**, then from **`cpp/`**:

```bat
%VCPKG_ROOT%\vcpkg.exe install --triplet x64-windows
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release
build\Release\chess-calc.exe
```

Use **`cpp/vcpkg.json`** for SDL2 versions. CMake copies **`assets/`** next to the executable.

## Stockfish (recommended)

For **best move** and **sidebar grade** (centipawn evaluation), install Stockfish locally or run the fetch script from **`cpp/`**:

```bash
python3 scripts/fetch_stockfish.py
```

Without it, **best move** can still use the **Lichess cloud-eval** fallback (needs `curl` and network). **Grading** requires a local engine binary.

## Run

Run the executable from the **`cpp/`** directory (or keep `assets/` next to the binary — CMake does this automatically).

## Usage

- **Drag pieces** on the board to move them anywhere (free placement) or to legal squares (makes a proper move)
- **Click a piece** to see its legal moves (highlighted in green)
- **Click a legal square** to make the move
- **Drag from palettes** (above/below board) to add pieces — for promotions or setup
- **Drag pieces onto palettes** to remove them from the board
- **Reset** — clear and start over
- **White to move** / **Black to move** — set whose turn and flip the board view
- **Calculate best move** — opening DB (trie + hash timing) or Stockfish; optional **SF grade** line in the sidebar
- **Sidebar** — turn, best move, possible moves, FEN (needs **SDL2_ttf**)

## Project structure

```
cpp/
├── src/               # SDL2 app, Stockfish UCI, opening DB, GUI
├── include/           # chess.hpp, headers
├── third_party/stockfish/  # optional local Stockfish (see README there)
├── scripts/           # PGN + Stockfish fetch helpers
├── vcpkg.json
├── assets/
└── CMakeLists.txt
```

## Dependencies

- **SDL2** — window, rendering, input  
- **SDL2_ttf** — sidebar text  
- **stb_image** — PNG loading (bundled in `include/stb_image.h`)  
- **chess.hpp** — move generation, FEN (bundled in `include/chess.hpp`)  

## Assets

Chess piece PNGs in `cpp/assets/white/` and `cpp/assets/black/` from [Sashité](https://sashite.dev/assets/chess/), CC0 (public domain).
