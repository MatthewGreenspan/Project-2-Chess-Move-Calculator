# Bundled Stockfish (optional)

The app checks **`third_party/stockfish/stockfish`**, then your **`PATH`**, then common paths such as **`/opt/homebrew/bin/stockfish`** (Homebrew on Apple Silicon).

**macOS (easiest):**

```bash
brew install stockfish
```

**Download into this folder** (bash + curl + tar; no Python):

```bash
cd cpp && sh scripts/fetch_stockfish.sh
```

The downloaded binary is gitignored; this README is tracked.

**Windows:** install from [stockfishchess.org](https://stockfishchess.org/download/) or run **`scripts/fetch_stockfish.sh`** from **Git Bash**.
