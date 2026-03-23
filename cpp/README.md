# Chess Move Calculator (C++)

Native window app for chess puzzle setup. Drag pieces, use palettes for promotions, view best move and possible moves in the sidebar.

## Build

```bash
brew install sdl2 sdl2_ttf stockfish
cd cpp
make
```

**Important:** `sdl2_ttf` is required for the sidebar to show text (White/Black to move, best move, possible moves). Without it, the sidebar will be empty.

## Run

```bash
./chess-calc
```

Stockfish is used for best-move analysis. If not installed, the first legal move is shown instead.
