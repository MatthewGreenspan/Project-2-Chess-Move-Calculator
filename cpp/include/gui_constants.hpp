#pragma once

#include <SDL2/SDL.h>

#if __has_include(<SDL2/SDL_ttf.h>)
#include <SDL2/SDL_ttf.h>
#define HAVE_TTF 1
#else
#define HAVE_TTF 0
#endif

using namespace std;
// ui sizing stuff all kept in one place
inline constexpr int SQUARE_SIZE = 72;
inline constexpr int BOARD_SIZE = 8 * SQUARE_SIZE;
inline constexpr int PALETTE_HEIGHT = 56;
inline constexpr int BUTTON_HEIGHT = 36;
inline constexpr int PADDING = 16;
inline constexpr int SIDEBAR_WIDTH = 300;
inline constexpr int BOARD_AREA_W = BOARD_SIZE + PADDING * 2;
inline constexpr int WINDOW_W = BOARD_AREA_W + SIDEBAR_WIDTH;
inline constexpr int WINDOW_H =
    PALETTE_HEIGHT + PADDING + BOARD_SIZE + PALETTE_HEIGHT + PADDING + BUTTON_HEIGHT + PADDING * 2 + 56;

inline constexpr Uint32 COLOR_BOARD_LIGHT = 0xf0d9b5ff;
inline constexpr Uint32 COLOR_BOARD_DARK = 0xb58863ff;
inline constexpr Uint32 COLOR_HIGHLIGHT = 0x7eb8da80;
inline constexpr Uint32 COLOR_LEGAL = 0x5a9b5a80;

// file names used when loading piece images
inline constexpr const char* PIECE_NAMES[] = {"king", "queen", "rook", "bishop", "knight", "pawn"};
inline constexpr const char* PIECE_NAMES_CAP[] = {"Pawn", "Knight", "Bishop", "Rook", "Queen", "King"};
