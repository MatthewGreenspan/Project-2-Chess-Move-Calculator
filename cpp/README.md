# Chess Move Calculator (C++)

Native SDL2 app: puzzle setup, palettes, Stockfish / Lichess best move, sidebar text.

## Dependencies (all platforms)

- **SDL2** — window, rendering, input  
- **SDL2_ttf** — sidebar text (without it, the sidebar is blank)  
- **CMake** 3.16+  

---

## Windows build (step by step)

### 1. Install tools

- **[Visual Studio 2022](https://visualstudio.microsoft.com/)** (or **Build Tools**) with workload **“Desktop development with C++”** (MSVC compiler, Windows SDK).
- **[CMake](https://cmake.org/download/)** — add to PATH during setup, or use the VS bundled CMake.
- **[Git for Windows](https://git-scm.com/download/win)** — gives you `git` and **`curl.exe`** on PATH (needed for the Lichess fallback if Stockfish is missing).

### 2. Install vcpkg (once per machine)

In PowerShell or **x64 Native Tools Command Prompt for VS** (recommended):

```powershell
cd C:\dev
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

Set an environment variable so CMake can find the toolchain (User or System **VCPKG_ROOT**):

`C:\dev\vcpkg` (or wherever you cloned it).

### 3. Build this project

From the repo’s **`cpp`** folder (where **`vcpkg.json`** lives):

```bat
cd path\to\Project-2-Chess-Move-Calculator\cpp
```

Install SDL2 + SDL2_ttf via the manifest (same versions for everyone):

```bat
%VCPKG_ROOT%\vcpkg.exe install --triplet x64-windows
```

Configure and build with MSVC:

```bat
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release
```

Run:

```bat
build\Release\chess-calc.exe
```

`cmake` copies **`assets`** next to **`chess-calc.exe`** automatically.

The build also copies **all DLLs from the vcpkg `bin` folder** next to `chess-calc.exe` (SDL2, SDL2_ttf, FreeType, zlib, etc.). That avoids Windows errors like **“The specified module could not be found”** when loading `SDL2_ttf.dll`. Always run the `.exe` from that same folder (e.g. `build\Release\`), or keep those DLLs beside the exe if you move it.

### 4. Windows extras (optional but useful)

| What | Why |
|------|-----|
| **Stockfish** | Best-move analysis. Download from [stockfishchess.org](https://stockfishchess.org/download/), unzip, add the folder containing `stockfish.exe` to **PATH**, or put `stockfish.exe` somewhere already on PATH. |
| **`curl.exe`** | Lichess API fallback. Usually comes with **Git for Windows**; ensure Git’s `usr\bin` or equivalent is on PATH. |

---

## macOS / Linux (pkg-config)

```bash
# macOS
brew install sdl2 sdl2_ttf pkg-config cmake

# Debian/Ubuntu
# sudo apt install libsdl2-dev libsdl2-ttf-dev pkg-config cmake

cd cpp
cmake -B build -S .
cmake --build build
./build/chess-calc
```

---

## Without pkg-config (macOS / Linux only)

If `pkg-config` is missing, point CMake at SDL2’s install prefix:

```bash
cmake -B build -S . -DCMAKE_PREFIX_PATH=/path/to/sdl2/prefix
```

---

## Makefile (Unix only)

```bash
cd cpp
make
./chess-calc
```

---

## Run notes

- Sidebar needs **SDL2_ttf** or text is hidden.  
- **Stockfish** on PATH when possible; otherwise **Lichess** via **curl** (see Windows table above).  
- Assets are copied next to the executable by CMake’s `POST_BUILD` step.

### Display / aspect ratio (HiDPI, Retina, fractional scaling)

The app uses a **fixed logical size** and **integer scaling** so the board should not look stretched. If something still looks wrong after a **full rebuild** (`make clean && make` or a clean CMake build):

1. Confirm your partner is running a **newly built** binary (not an old copy).
2. **macOS / Retina:** run with HiDPI disabled for a 1:1 pixel path:  
   `CHESS_CALC_DISABLE_HIGHDPI=1 ./chess-calc`
3. **Windows:** same variable in an environment variable or:  
   `set CHESS_CALC_DISABLE_HIGHDPI=1` then run `chess-calc.exe` from the folder that contains the DLLs.
