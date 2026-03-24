#!/usr/bin/env python3
"""
Fetch rated games from Lichess using the official HTTP API (not web scraping).
Filters to games where BOTH [WhiteElo] and [BlackElo] are in [min_elo, max_elo].

Usage:
  python3 fetch_lichess_sample_pgn.py --out ../lichess_games.pgn \\
      --users DrNykterstein --users SomeUser --min 1800 --max 2500 --max-games 80

If few games match (e.g. GMs are 3000+), pass usernames of ~2000-rated players,
or use build_sample_pgn.py for an offline synthetic file.
"""
from __future__ import annotations

import argparse
import re
import sys
import time
import urllib.request

USER_AGENT = "ChessMoveCalculator/1.0 (https://github.com/MatthewGreenspan/Project-2-Chess-Move-Calculator)"


def fetch_pgn(username: str, max_games: int) -> str:
    url = f"https://lichess.org/api/games/user/{username}?max={max_games}&rated=true"
    req = urllib.request.Request(
        url,
        headers={
            "User-Agent": USER_AGENT,
            "Accept": "application/x-chess-pgn",
        },
    )
    with urllib.request.urlopen(req, timeout=60) as r:
        return r.read().decode("utf-8", errors="replace")


def parse_elo(headers: str):
    wm = re.search(r'\[WhiteElo\s+"(\d+)"\]', headers)
    bm = re.search(r'\[BlackElo\s+"(\d+)"\]', headers)
    w = int(wm.group(1)) if wm else None
    b = int(bm.group(1)) if bm else None
    return w, b


def split_games(pgn_blob: str) -> list[str]:
    parts = re.split(r"\n\n(?=\[Event )", pgn_blob.strip())
    if parts and not parts[0].startswith("[Event "):
        parts = [p for p in parts if p.strip().startswith("[Event ")]
    return [p.strip() for p in parts if p.strip()]


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default="lichess_games.pgn", help="Output path")
    ap.add_argument("--users", action="append", required=True, help="Lichess username (repeatable)")
    ap.add_argument("--min", dest="min_elo", type=int, default=1800)
    ap.add_argument("--max", dest="max_elo", type=int, default=2500)
    ap.add_argument("--per-user", type=int, default=120, help="Max games to request per user")
    ap.add_argument("--max-games", type=int, default=100, help="Max games to write total")
    args = ap.parse_args()

    kept: list[str] = []
    for user in args.users:
        time.sleep(1.2)  # be polite to API rate limits
        try:
            blob = fetch_pgn(user, args.per_user)
        except Exception as e:
            print(f"skip {user}: {e}", file=sys.stderr)
            continue
        for g in split_games(blob):
            head = g.split("\n\n", 1)[0] if "\n\n" in g else g
            w, b = parse_elo(head)
            if w is None or b is None:
                continue
            if args.min_elo <= w <= args.max_elo and args.min_elo <= b <= args.max_elo:
                kept.append(g)
                if len(kept) >= args.max_games:
                    break
        if len(kept) >= args.max_games:
            break

    if not kept:
        print("No games matched the Elo filter. Try different --users (club-level players) or run build_sample_pgn.py.", file=sys.stderr)
        sys.exit(1)

    with open(args.out, "w", encoding="utf-8") as f:
        f.write("\n\n".join(kept) + "\n")
    print(f"Wrote {len(kept)} games to {args.out}")


if __name__ == "__main__":
    main()
