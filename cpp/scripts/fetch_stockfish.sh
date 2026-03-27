#!/usr/bin/env bash
# Download official Stockfish into cpp/third_party/stockfish/ (no Python).
# Requires: bash, curl, tar (and unzip on Windows/Git Bash for .zip).
# Usage: from cpp/  →  sh scripts/fetch_stockfish.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CPP_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
DEST="$CPP_DIR/third_party/stockfish"
API="https://api.github.com/repos/official-stockfish/Stockfish/releases/latest"
USER_AGENT="ChessMoveCalculator-fetch-stockfish/1.0"

OS="$(uname -s)"
MACH="$(uname -m 2>/dev/null || echo unknown)"

pick_asset() {
  case "$OS" in
    Darwin)
      case "$MACH" in
        arm64|aarch64) echo "stockfish-macos-m1-apple-silicon.tar" ;;
        *) echo "stockfish-macos-x86-64-avx2.tar" ;;
      esac
      ;;
    Linux)
      echo "stockfish-ubuntu-x86-64-avx2.tar"
      ;;
    MINGW*|MSYS*|CYGWIN_NT*)
      echo "stockfish-windows-x86-64-avx2.zip"
      ;;
    *)
      echo ""
      ;;
  esac
}

ASSET_NAME="$(pick_asset)"
if [[ -z "$ASSET_NAME" ]]; then
  echo "Unsupported OS: $OS — install Stockfish from https://stockfishchess.org/download/" >&2
  exit 1
fi

mkdir -p "$DEST"

# Keep README.md; remove other previous content
for n in "$DEST"/*; do
  [[ -e "$n" ]] || continue
  base=$(basename "$n")
  [[ "$base" == "README.md" ]] && continue
  rm -rf "$n"
done

JSON=$(curl -sL -m 120 -H "User-Agent: $USER_AGENT" "$API")
URL=$(echo "$JSON" | grep -oE 'https://[^"]+' | grep "$ASSET_NAME" | head -1 || true)
if [[ -z "$URL" ]]; then
  echo "Could not resolve download URL for $ASSET_NAME" >&2
  exit 1
fi

ARCHIVE="$DEST/$ASSET_NAME"
echo "Downloading $ASSET_NAME ..."
curl -sL -m 600 -H "User-Agent: $USER_AGENT" -o "$ARCHIVE" "$URL"

EXTRACT="$DEST/.extract_tmp"
mkdir -p "$EXTRACT"
if [[ "$ASSET_NAME" == *.zip ]]; then
  unzip -q -o "$ARCHIVE" -d "$EXTRACT"
else
  tar -xf "$ARCHIVE" -C "$EXTRACT"
fi

# Prefer plain names, then stockfish-<arch> (official release layout)
EXE=$(find "$EXTRACT" -type f \( -name 'stockfish.exe' -o -name 'stockfish' \) 2>/dev/null | head -1)
if [[ -z "$EXE" ]]; then
  EXE=$(find "$EXTRACT" -type f -name 'stockfish-*' ! -name '*.md' 2>/dev/null | head -1)
fi

if [[ -z "$EXE" ]]; then
  echo "Could not find stockfish binary in archive." >&2
  rm -rf "$EXTRACT"
  exit 1
fi

if [[ "$ASSET_NAME" == *.zip ]]; then
  FINAL="$DEST/stockfish.exe"
else
  FINAL="$DEST/stockfish"
fi

cp -f "$EXE" "$FINAL"
chmod 755 "$FINAL" 2>/dev/null || true
rm -rf "$EXTRACT" "$ARCHIVE"
echo "Installed: $FINAL"
echo "Or use: brew install stockfish (macOS) — app also checks PATH."
