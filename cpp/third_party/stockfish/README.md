# Bundled Stockfish (optional)

The app looks for a `stockfish` binary here **before** searching your system `PATH`.

From the **`cpp/`** directory, run:

```bash
python3 scripts/fetch_stockfish.py
```

This downloads the latest official [Stockfish](https://github.com/official-stockfish/Stockfish) release for your OS into this folder. The binary is gitignored; only this README is tracked.

Alternatively: `brew install stockfish` (macOS) or install from [stockfishchess.org](https://stockfishchess.org/download/) and ensure `stockfish` is on `PATH`.
