#!/bin/sh
# Emit compile_commands.json for clangd / IDE (CMake only — no Python).
# Run from cpp/:  sh gen_compile_commands.sh

set -e
ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"
if ! command -v cmake >/dev/null 2>&1; then
  echo "cmake not found. Install CMake, or configure your IDE manually." >&2
  exit 1
fi
cmake -B build -S . -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
cp -f build/compile_commands.json "$ROOT/compile_commands.json"
echo "Wrote $ROOT/compile_commands.json"
