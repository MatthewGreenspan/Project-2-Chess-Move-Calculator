#!/usr/bin/env python3
"""Emit compile_commands.json for clangd / IDE (run from cpp/: python3 generate_compile_commands.py)."""
import json
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.abspath(__file__))
SOURCES = [
    "src/main.cpp",
    "src/sdl_helpers.cpp",
    "src/stockfish.cpp",
    "src/opening_db.cpp",
    "src/chess_gui_helpers.cpp",
    "src/best_move.cpp",
    "src/opening_suggestion.cpp",
    "src/gui_assets.cpp",
    "src/gui_render.cpp",
    "src/gui_input.cpp",
    "src/app_actions.cpp",
]

BASE = [
    "clang++",
    "-std=c++17",
    "-O2",
    "-DNDEBUG",
    "-I",
    os.path.join(ROOT, "include"),
    "-I",
    os.path.join(ROOT, "src"),
]


def pkg_config_cflags():
    try:
        out = subprocess.check_output(
            ["pkg-config", "--cflags", "sdl2", "SDL2_ttf"],
            cwd=ROOT,
            stderr=subprocess.DEVNULL,
            text=True,
        )
        return out.split()
    except (subprocess.CalledProcessError, FileNotFoundError):
        pass
    for p in ("/opt/homebrew/include", "/usr/local/include"):
        if os.path.isdir(p):
            return ["-I" + p]
    return []


def main():
    flags = BASE + pkg_config_cflags()
    entries = []
    for rel in SOURCES:
        path = os.path.join(ROOT, rel)
        cmd = flags + ["-c", path]
        entries.append(
            {
                "directory": ROOT,
                "command": " ".join(cmd),
                "file": path,
            }
        )
    out = os.path.join(ROOT, "compile_commands.json")
    with open(out, "w", encoding="utf-8") as f:
        json.dump(entries, f, indent=2)
        f.write("\n")
    print("Wrote", out, file=sys.stderr)


if __name__ == "__main__":
    main()
