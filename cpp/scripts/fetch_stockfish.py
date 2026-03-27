#!/usr/bin/env python3
"""
Download the official Stockfish binary for this machine into cpp/third_party/stockfish/.
Requires: Python 3.8+, urllib (stdlib), network.

Run from repo root:
  python3 cpp/scripts/fetch_stockfish.py

Or from cpp/:
  python3 scripts/fetch_stockfish.py
"""
import json
import os
import platform
import shutil
import sys
import tarfile
import urllib.request
import zipfile
from typing import List, Optional

API = "https://api.github.com/repos/official-stockfish/Stockfish/releases/latest"
USER_AGENT = "ChessMoveCalculator-fetch-stockfish/1.0"


def out_dir() -> str:
    script = os.path.dirname(os.path.abspath(__file__))
    cpp = os.path.dirname(script)
    d = os.path.join(cpp, "third_party", "stockfish")
    os.makedirs(d, exist_ok=True)
    return d


def pick_asset(names: List[str]) -> Optional[str]:
    s = platform.system()
    mach = platform.machine().lower()
    is_arm = "arm" in mach or "aarch64" in mach
    if s == "Darwin":
        if is_arm:
            return next((n for n in names if n == "stockfish-macos-m1-apple-silicon.tar"), None)
        return next((n for n in names if n == "stockfish-macos-x86-64-avx2.tar"), None)
    if s == "Linux":
        return next((n for n in names if n == "stockfish-ubuntu-x86-64-avx2.tar"), None)
    if s == "Windows" or os.name == "nt":
        return next((n for n in names if n == "stockfish-windows-x86-64-avx2.zip"), None)
    return None


def find_executable(root: str) -> Optional[str]:
    for dirpath, _, files in os.walk(root):
        for f in files:
            if f == "stockfish" or f.lower() == "stockfish.exe":
                p = os.path.join(dirpath, f)
                if os.path.isfile(p):
                    return p
    return None


def main() -> None:
    req = urllib.request.Request(API, headers={"User-Agent": USER_AGENT})
    with urllib.request.urlopen(req, timeout=60) as r:
        data = json.loads(r.read().decode())
    assets = [a["name"] for a in data.get("assets", [])]
    browser = {a["name"]: a["browser_download_url"] for a in data.get("assets", [])}
    name = pick_asset(assets)
    if not name:
        print("No matching Stockfish asset for this OS. Install via: brew install stockfish", file=sys.stderr)
        sys.exit(1)
    url = browser[name]
    dest = out_dir()
    archive = os.path.join(dest, name)
    print(f"Downloading {name} ...")
    urllib.request.urlretrieve(url, archive)

    # Clean old binaries
    for n in os.listdir(dest):
        p = os.path.join(dest, n)
        if n != name and os.path.isfile(p):
            os.remove(p)
        elif n != name and os.path.isdir(p) and n != "stockfish-src":
            shutil.rmtree(p, ignore_errors=True)

    if name.endswith(".tar"):
        with tarfile.open(archive, "r", format=tarfile.GNU_FORMAT) as tf:
            tf.extractall(dest)
    elif name.endswith(".zip"):
        with zipfile.ZipFile(archive, "r") as zf:
            zf.extractall(dest)

    exe = find_executable(dest)
    if not exe:
        print("Extracted but could not find stockfish binary.", file=sys.stderr)
        sys.exit(1)
    final = os.path.join(dest, "stockfish.exe" if exe.endswith(".exe") else "stockfish")
    if os.path.abspath(exe) != os.path.abspath(final):
        if os.path.exists(final):
            os.remove(final)
        shutil.move(exe, final)
    os.chmod(final, 0o755)
    try:
        os.remove(archive)
    except OSError:
        pass
    print(f"Installed: {final}")
    print("Run chess-calc from the cpp/ folder or ensure the app looks in third_party/stockfish/.")


if __name__ == "__main__":
    main()
