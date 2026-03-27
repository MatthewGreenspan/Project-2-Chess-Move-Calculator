# Bundled Stockfish (optional)

The app checks **`third_party/stockfish/stockfish`**, then your **`PATH`**, then common paths such as **`/opt/homebrew/bin/stockfish`** (Homebrew on Apple Silicon).

**macOS (easiest):**

```bash
brew install stockfish
```

**Project-local copy** (downloads a release into this folder; uses `curl` for HTTPS):

```bash
cd cpp && python3 scripts/fetch_stockfish.py
```

The downloaded `stockfish` binary is gitignored; this README is tracked.

**Windows / Linux:** install from [stockfishchess.org](https://stockfishchess.org/download/) or use `fetch_stockfish.py` from `cpp/`.
