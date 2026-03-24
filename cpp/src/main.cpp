#include "app.hpp"
#include "app_actions.hpp"
#include "best_move.hpp"
#include "chess_gui_helpers.hpp"
#include "gui_assets.hpp"
#include "gui_constants.hpp"
#include "gui_input.hpp"
#include "gui_render.hpp"
#include "opening_db.hpp"
#include "sdl_helpers.hpp"
#include <SDL2/SDL.h>
#include <string>

#if HAVE_TTF
#include <SDL2/SDL_ttf.h>
#endif

using namespace chess;
using namespace chess_gui;

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  loadOpeningDatabase();

  sdlApplyHintsBeforeInit();
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_Log("SDL_Init failed: %s", SDL_GetError());
    return 1;
  }
#if HAVE_TTF
  if (TTF_Init() != 0) SDL_Log("TTF_Init failed - no text");
#endif

  App app;
  app.assetPath = findAssetPath();
  boardToArray(app.board, app.pieces);

  app.window = SDL_CreateWindow("Chess Move Calculator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_W,
                                WINDOW_H, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
  if (!app.window) {
    SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
#if HAVE_TTF
    TTF_Quit();
#endif
    SDL_Quit();
    return 1;
  }

  SDL_SetWindowMinimumSize(app.window, WINDOW_W, WINDOW_H);
  SDL_SetWindowMaximumSize(app.window, WINDOW_W, WINDOW_H);

  app.renderer = SDL_CreateRenderer(app.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!app.renderer) {
    SDL_DestroyWindow(app.window);
#if HAVE_TTF
    TTF_Quit();
#endif
    SDL_Quit();
    return 1;
  }

  SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
  sdlSetupRendererLogical(app.renderer, WINDOW_W, WINDOW_H);

  if (!loadAssets(app)) {
    SDL_Log("Failed to create piece textures.");
    SDL_DestroyRenderer(app.renderer);
    SDL_DestroyWindow(app.window);
#if HAVE_TTF
    TTF_Quit();
#endif
    SDL_Quit();
    return 1;
  }

  while (!app.quit) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) app.quit = true;
      else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        app.mouseX = app.mouseDownX = e.button.x;
        app.mouseY = app.mouseDownY = e.button.y;
        int sq = getSquareAt(app, e.button.x, e.button.y);
        int pal = getPalettePieceAt(app, e.button.x, e.button.y);

        int sideBtn = getSidebarButtonAt(e.button.x, e.button.y);
        if (sideBtn == 1) {
          std::string fen = app.board.getFen();
          size_t pos = fen.find(' ');
          if (pos != std::string::npos) fen = fen.substr(0, pos) + " w" + fen.substr(pos + 2);
          app.board.setFen(fen);
          app.boardFlipped = false;
          boardToArray(app.board, app.pieces);
          updateLegalMoves(app);
          clearBestMoveDisplay(app);
        } else if (sideBtn == 2) {
          std::string fen = app.board.getFen();
          size_t pos = fen.find(' ');
          if (pos != std::string::npos) fen = fen.substr(0, pos) + " b" + fen.substr(pos + 2);
          app.board.setFen(fen);
          app.boardFlipped = true;
          boardToArray(app.board, app.pieces);
          updateLegalMoves(app);
          clearBestMoveDisplay(app);
        } else if (sideBtn == 3) {
          updateBestMove(app, true);
        } else {
          int btnY = PADDING + PALETTE_HEIGHT + PADDING + BOARD_SIZE + PALETTE_HEIGHT + PADDING * 2;
          if (e.button.y >= btnY && e.button.y < btnY + BUTTON_HEIGHT && e.button.x >= PADDING && e.button.x < PADDING + 120) {
            app.board = Board(constants::STARTPOS);
            boardToArray(app.board, app.pieces);
            app.boardFlipped = false;
            app.selectedSquare = -1;
            app.legalMoveSquares.clear();
            app.movesPlayed.clear();
            clearBestMoveDisplay(app);
          } else if (pal >= 0) {
            app.dragFromPalette = pal;
            app.dragFrom = -1;
          } else if (sq >= 0) {
            if (app.pieces[sq] != Piece::NONE) {
              app.dragFrom = sq;
              app.dragFromPalette = -1;
              app.selectedSquare = sq;
              updateLegalMoves(app);
            } else if (app.selectedSquare >= 0) {
              if (!tryLegalMove(app, app.selectedSquare, sq)) {
                app.selectedSquare = -1;
                app.legalMoveSquares.clear();
              }
            }
          }
        }
      } else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
        app.mouseX = e.button.x;
        app.mouseY = e.button.y;
        int dx = e.button.x - app.mouseDownX;
        int dy = e.button.y - app.mouseDownY;
        bool wasDrag = (dx * dx + dy * dy) > 25;

        int sq = getSquareAt(app, e.button.x, e.button.y);
        if (wasDrag) {
          if (inBoard(app, e.button.x, e.button.y) && sq >= 0) {
            if (app.dragFrom >= 0) {
              if (!tryLegalMove(app, app.dragFrom, sq)) doDrop(app, sq);
            } else if (app.dragFromPalette >= 0) {
              doDrop(app, sq);
            }
          } else if (app.dragFrom >= 0) {
            doRemove(app);
          }
          app.dragFrom = -1;
          app.dragFromPalette = -1;
        } else if (app.dragFrom >= 0 && sq >= 0 && sq != app.dragFrom) {
          if (!tryLegalMove(app, app.dragFrom, sq)) doDrop(app, sq);
          app.dragFrom = -1;
          app.dragFromPalette = -1;
        } else if (app.dragFromPalette >= 0 && sq >= 0) {
          doDrop(app, sq);
          app.dragFrom = -1;
          app.dragFromPalette = -1;
        } else if (app.dragFrom >= 0 && sq == app.dragFrom) {
          app.dragFrom = -1;
        } else {
          app.dragFrom = -1;
          app.dragFromPalette = -1;
        }
      } else if (e.type == SDL_MOUSEMOTION) {
        app.mouseX = e.motion.x;
        app.mouseY = e.motion.y;
      }
    }
    render(app);
    SDL_Delay(16);
  }

  for (int c = 0; c < 2; c++)
    for (int p = 0; p < 6; p++)
      if (app.pieceTex[c][p]) SDL_DestroyTexture(app.pieceTex[c][p]);
#if HAVE_TTF
  if (app.fontSmall && app.fontSmall != app.font) TTF_CloseFont(app.fontSmall);
  if (app.font) TTF_CloseFont(app.font);
  TTF_Quit();
#endif
  SDL_DestroyRenderer(app.renderer);
  SDL_DestroyWindow(app.window);
  SDL_Quit();
  return 0;
}
