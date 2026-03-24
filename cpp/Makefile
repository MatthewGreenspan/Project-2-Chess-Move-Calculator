# Chess Move Calculator - Native build
# macOS: brew install sdl2 sdl2_ttf
# Sidebar text requires SDL2_ttf!

CXX = clang++
CXXFLAGS = -std=c++17 -O2 -DNDEBUG -I include -I src

# Try pkg-config first, fallback to Homebrew paths
PKG_CONFIG := $(shell which pkg-config 2>/dev/null)
ifdef PKG_CONFIG
  SDL_CFLAGS := $(shell pkg-config --cflags sdl2 2>/dev/null)
  SDL_LIBS := $(shell pkg-config --libs sdl2 2>/dev/null)
  ifneq ($(shell pkg-config --exists SDL2_ttf 2>/dev/null && echo 1),)
    SDL_CFLAGS += $(shell pkg-config --cflags SDL2_ttf 2>/dev/null)
    SDL_LIBS += $(shell pkg-config --libs SDL2_ttf 2>/dev/null)
    HAVE_TTF := 1
  endif
endif

ifeq ($(SDL_CFLAGS),)
  HOMEBREW_PREFIX := $(shell brew --prefix 2>/dev/null || echo /opt/homebrew)
  SDL_CFLAGS = -I$(HOMEBREW_PREFIX)/include
  SDL_LIBS = -L$(HOMEBREW_PREFIX)/lib -lSDL2
  ifneq ($(wildcard $(HOMEBREW_PREFIX)/lib/libSDL2_ttf*.dylib),)
    SDL_LIBS += -lSDL2_ttf
    HAVE_TTF := 1
  endif
  ifneq ($(wildcard $(HOMEBREW_PREFIX)/lib/libSDL2_ttf*.so*),)
    SDL_LIBS += -lSDL2_ttf
    HAVE_TTF := 1
  endif
endif

ifeq ($(HAVE_TTF),)
  $(warning SDL2_ttf not found - sidebar will be empty. Install with: brew install sdl2_ttf)
endif

# Rebuild when headers change (only .cpp files are passed to the linker)
SRCS := src/main.cpp src/sdl_helpers.cpp src/stockfish.cpp src/opening_db.cpp src/chess_gui_helpers.cpp \
	src/best_move.cpp src/gui_assets.cpp src/gui_render.cpp src/gui_input.cpp src/app_actions.cpp
HDRS := $(wildcard include/*.hpp src/*.h)

chess-calc: $(SRCS) $(HDRS)
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) -o $@ $(SRCS) $(SDL_LIBS)

run: chess-calc
	./chess-calc

clean:
	rm -f chess-calc

# Regenerate compile_commands.json for clangd / IDE (run after changing deps or paths)
compile_commands.json:
	python3 generate_compile_commands.py

compile_commands: compile_commands.json

.PHONY: run clean compile_commands
