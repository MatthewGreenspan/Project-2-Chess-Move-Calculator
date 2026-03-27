#!/usr/bin/env python3
"""
Download the official Stockfish binary into cpp/third_party/stockfish/.

Uses curl for HTTPS when available (avoids broken Python SSL on some macOS installs).

On macOS, `brew install stockfish` is often simpler — the app also checks PATH and
/opt/homebrew/bin/stockfish.

Run from repo root:
  python3 cpp/scripts/fetch_stockfish.py

Or from cpp/:
  python3 scripts/fetch_stockfish.py
"""
import json
import os
import platform
import shutil
import subprocess
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
    """Locate engine binary (name may be stockfish, stockfish.exe, or stockfish-<arch>)."""
    for dirpath, _, files in os.walk(root):
        for f in files:
            if f.lower() == "stockfish.exe":
                p = os.path.join(dirpath, f)
                if os.path.isfile(p):
                    return p
            if not f.startswith("stockfish"):
                continue
            if f.endswith((".md", ".txt", ".cff", ".py")):
                continue
            p = os.path.join(dirpath, f)
            if not os.path.isfile(p):
                continue
            if f.lower().endswith(".exe") or os.access(p, os.X_OK):
                return p
    return None


def fetch_url_bytes(url: str) -> bytes:
    curl = "curl.exe" if platform.system() == "Windows" else "curl"
    try:
        r = subprocess.run(
            [curl, "-sL", "-m", "120", "-A", USER_AGENT, url],
            capture_output=True,
            timeout=130,
        )
        if r.returncode == 0 and r.stdout:
            return r.stdout
    except (OSError, subprocess.TimeoutExpired):
        pass
    req = urllib.request.Request(url, headers={"User-Agent": USER_AGENT})
    with urllib.request.urlopen(req, timeout=120) as resp:
        return resp.read()


def download_to_file(url: str, path: str) -> None:
    curl = "curl.exe" if platform.system() == "Windows" else "curl"
    try:
        subprocess.run(
            [curl, "-sL", "-m", "600", "-A", USER_AGENT, "-o", path, url],
            check=True,
            timeout=610,
        )
        return
    except (OSError, subprocess.CalledProcessError, subprocess.TimeoutExpired):
        pass
    urllib.request.urlretrieve(url, path)


def main() -> None:
    raw = fetch_url_bytes(API)
    data = json.loads(raw.decode("utf-8", errors="replace"))
    assets = [a["name"] for a in data.get("assets", [])]
    browser = {a["name"]: a["browser_download_url"] for a in data.get("assets", [])}
    name = pick_asset(assets)
    if not name:
        print("No matching Stockfish asset for this OS. Try: brew install stockfish", file=sys.stderr)
        sys.exit(1)
    url = browser[name]
    dest = out_dir()
    archive = os.path.join(dest, name)

    # Clear old downloads (keep README.md) before fetching the new archive.
    for n in os.listdir(dest):
        if n == "README.md":
            continue
        p = os.path.join(dest, n)
        try:
            if os.path.isfile(p):
                os.remove(p)
            elif os.path.isdir(p):
                shutil.rmtree(p, ignore_errors=True)
        except OSError:
            pass

    print(f"Downloading {name} ...")
    download_to_file(url, archive)

    # Avoid extracting into dest/ — archives
    # often contain a "stockfish/" folder that would collide with our output file "stockfish".

    extract_root = os.path.join(dest, ".extract_tmp")
    os.makedirs(extract_root, exist_ok=True)
    if name.endswith(".tar"):
        with tarfile.open(archive, "r", format=tarfile.GNU_FORMAT) as tf:
            tf.extractall(extract_root)
    elif name.endswith(".zip"):
        with zipfile.ZipFile(archive, "r") as zf:
            zf.extractall(extract_root)

    exe = find_executable(extract_root)
    if not exe:
        shutil.rmtree(extract_root, ignore_errors=True)
        print("Extracted but could not find stockfish binary.", file=sys.stderr)
        sys.exit(1)
    final = os.path.join(dest, "stockfish.exe" if exe.lower().endswith(".exe") else "stockfish")
    shutil.copy2(exe, final)
    os.chmod(final, 0o755)
    shutil.rmtree(extract_root, ignore_errors=True)
    try:
        os.remove(archive)
    except OSError:
        pass
    print(f"Installed: {final}")
    print("Run chess-calc from the cpp/ folder or use stockfish on PATH (e.g. brew).")


if __name__ == "__main__":
    main()
