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

using namespace chess;

struct App {
  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;
  SDL_Texture* pieceTex[2][6] = {{nullptr}};
  std::vector<std::string> movesPlayed;
#if HAVE_TTF
  TTF_Font* font = nullptr;
  TTF_Font* fontSmall = nullptr;
#endif
  Board board{constants::STARTPOS};
  std::array<Piece, 64> pieces;
  bool boardFlipped = false;
  std::string assetPath;
  int dragFrom = -1;
  int dragFromPalette = -1;
  int mouseX = 0, mouseY = 0;
  int mouseDownX = 0, mouseDownY = 0;
  int selectedSquare = -1;
  std::vector<Square> legalMoveSquares;
  std::string bestMoveUci;
  std::string bestMoveSan;
  std::string bestMoveEnglish;
  std::string lastAnalyzedFen;
  int bestMoveFrom = -1;
  int bestMoveTo = -1;
  bool showBestMoveArrow = false;
  bool quit = false;
};