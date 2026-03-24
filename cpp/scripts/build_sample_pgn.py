#!/usr/bin/env python3
"""
Generate lichess_games.pgn: many full games (legal moves) for the opening trie.

Requires: pip install -r requirements.txt   (python-chess)

Each game is played with random legal moves until the position is terminal (mate,
stalemate, draw) or a safety cap on half-moves is reached.
Elo headers are random in 1800–2500 (both players). Not real Lichess games — use
fetch_lichess_sample_pgn.py for API exports.
"""
from __future__ import annotations

import argparse
import random

try:
    import chess
except ImportError as e:
    raise SystemExit(
        "Missing dependency: pip install chess\n"
        "  cd cpp/scripts && pip install -r requirements.txt"
    ) from e


def play_random_game(rng: random.Random, max_plies: int) -> tuple[str, list[str]]:
    board = chess.Board()
    sans: list[str] = []
    while not board.is_game_over() and len(sans) < max_plies:
        moves = list(board.legal_moves)
        mv = rng.choice(moves)
        sans.append(board.san(mv))
        board.push(mv)
    return board.result(), sans


def format_movetext(sans: list[str]) -> str:
    parts: list[str] = []
    for i, san in enumerate(sans):
        if i % 2 == 0:
            parts.append(f"{i // 2 + 1}. {san}")
        else:
            parts.append(san)
    return " ".join(parts)


def main() -> None:
    p = argparse.ArgumentParser()
    p.add_argument("--out", default="lichess_games.pgn")
    p.add_argument("--games", type=int, default=120, help="Number of games")
    p.add_argument(
        "--max-plies",
        type=int,
        default=2000,
        help="Safety cap on half-moves (random games can be long; raise if many [Result \"*\"])",
    )
    p.add_argument("--seed", type=int, default=None)
    args = p.parse_args()
    rng = random.Random(args.seed)
    chunks: list[str] = []
    for g in range(args.games):
        w = rng.randint(1800, 2500)
        b = rng.randint(1800, 2500)
        res, sans = play_random_game(rng, args.max_plies)
        movetext = format_movetext(sans) + f" {res}"
        chunks.append(
            "\n".join(
                [
                    f'[Event "generated fullgame {g + 1}"]',
                    '[Site "https://lichess.org/sample"]',
                    f'[WhiteElo "{w}"]',
                    f'[BlackElo "{b}"]',
                    f'[Result "{res}"]',
                    "",
                    movetext,
                    "",
                ]
            )
        )
    with open(args.out, "w", encoding="utf-8") as f:
        f.write("\n".join(chunks))
    print(f"Wrote {args.games} games (up to {args.max_plies} plies each) to {args.out}")


if __name__ == "__main__":
    main()
