#pragma once

#include "chess.hpp"
#include "gui_constants.hpp"
#include <SDL2/SDL.h>
#include <array>
#include <string>
#include <vector>

#if HAVE_TTF
#include <SDL2/SDL_ttf.h>
#endif

using namespace std;
using namespace chess;

struct App {
  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;
  SDL_Texture* pieceTex[2][6] = {{nullptr}};
  vector<string> movesPlayed;
#if HAVE_TTF
  TTF_Font* font = nullptr;
  TTF_Font* fontSmall = nullptr;
#endif
  Board board{constants::STARTPOS};
  array<Piece, 64> pieces;
  bool boardFlipped = false;
  string assetPath;
  int dragFrom = -1;
  int dragFromPalette = -1;
  int mouseX = 0, mouseY = 0;
  int mouseDownX = 0, mouseDownY = 0;
  /** From getSidebarButtonAt: -1 none, 1 left turn btn, 2 right turn btn, 3 calculate. */
  int hoverSidebarButton = -1;
  int selectedSquare = -1;
  vector<Square> legalMoveSquares;
  string bestMoveUci;
  string bestMoveSan;
  string bestMoveEnglish;
  string openingLookupCompare;
  string trieTimingLine;
  string hashTimingLine;
  string openingTrieLine;
  string openingTrieSecondLine;
  string openingHashLine;
  string openingHashSecondLine;
  string openingScanLine;
  string moveGradeLine;
  string lastAnalyzedFen;
  string prefixGamesLine;
  int bestMoveFrom = -1;
  int bestMoveTo = -1;
  int secondBestMoveFrom = -1;
  int secondBestMoveTo = -1;
  bool showBestMoveArrow = false;
  bool showSecondBestArrow = false;
  bool quit = false;
};
