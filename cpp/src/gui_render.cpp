#include "gui_render.hpp"
#include "chess_gui_helpers.hpp"
#include "gui_constants.hpp"
#include <algorithm>
#include <cmath>

using namespace chess;
using namespace chess_gui;

static void drawPiece(SDL_Renderer* r, SDL_Texture* tex, int x, int y, int size) {
  if (!tex) return;
  SDL_Rect dst = {x, y, size, size};
  SDL_RenderCopy(r, tex, nullptr, &dst);
}

static void squareToPixel(const App& app, int boardY, int sqIndex, int& outX, int& outY) {
  int row = sqIndex / 8, col = sqIndex % 8;
  int dispRow = app.boardFlipped ? (7 - row) : row;
  outX = PADDING + col * SQUARE_SIZE + SQUARE_SIZE / 2;
  outY = boardY + dispRow * SQUARE_SIZE + SQUARE_SIZE / 2;
}

static void drawArrowTint(SDL_Renderer* r, int x1, int y1, int x2, int y2, Uint8 br, Uint8 bg, Uint8 bb) {
  float dx = (float)(x2 - x1), dy = (float)(y2 - y1);
  float len = std::sqrt(dx * dx + dy * dy);
  if (len < 4.0f) return;
  dx /= len;
  dy /= len;

  const float shaftEnd = len - 26.0f;
  if (shaftEnd < 2.0f) return;

  const int thick = 7;
  const float arrLen = 26.0f;
  const float wing = 14.0f;
  float perpX = -dy, perpY = dx;

  auto drawThickLine = [r](int xa, int ya, int xb, int yb, int w) {
    float dx = (float)(xb - xa), dy = (float)(yb - ya);
    float l = std::sqrt(dx * dx + dy * dy);
    if (l < 0.5f) return;
    dx /= l;
    dy /= l;
    float px = -dy, py = dx;
    for (int i = -w; i <= w; i++) {
      int ox = (int)(px * i), oy = (int)(py * i);
      SDL_RenderDrawLine(r, xa + ox, ya + oy, xb + ox, yb + oy);
    }
  };

  int sx2 = (int)(x1 + dx * shaftEnd), sy2 = (int)(y1 + dy * shaftEnd);
  float backX = (float)x2 - dx * arrLen, backY = (float)y2 - dy * arrLen;
  int ax = (int)(backX + perpX * wing), ay = (int)(backY + perpY * wing);
  int bx = (int)(backX - perpX * wing), by = (int)(backY - perpY * wing);

  auto dim = [](Uint8 v, int d) -> Uint8 { return (Uint8)(v > (Uint8)d ? v - (Uint8)d : 0); };
  SDL_SetRenderDrawColor(r, dim(br, 50), dim(bg, 50), dim(bb, 50), 255);
  drawThickLine(x1, y1, sx2, sy2, thick + 3);
  drawThickLine(x2, y2, ax, ay, 4);
  drawThickLine(x2, y2, bx, by, 4);
  drawThickLine(ax, ay, bx, by, 4);

  SDL_SetRenderDrawColor(r, br, bg, bb, 255);
  drawThickLine(x1, y1, sx2, sy2, thick);
  SDL_RenderDrawLine(r, x2, y2, ax, ay);
  SDL_RenderDrawLine(r, x2, y2, bx, by);
  SDL_RenderDrawLine(r, ax, ay, bx, by);

  SDL_SetRenderDrawColor(r, (Uint8)(std::min(255, (int)br + 25)), (Uint8)(std::min(255, (int)bg + 25)),
                         (Uint8)(std::min(255, (int)bb + 25)), 255);
  drawThickLine(x1, y1, sx2, sy2, thick - 3);
}

#if HAVE_TTF
static void fillTurnButton(SDL_Renderer* r, const SDL_Rect& rect, bool active, bool hover) {
  if (active)
    SDL_SetRenderDrawColor(r, hover ? (Uint8)0x36 : (Uint8)0x2a, hover ? (Uint8)0x62 : (Uint8)0x5a,
                           hover ? (Uint8)0xa8 : (Uint8)0x90, 255);
  else if (hover)
    SDL_SetRenderDrawColor(r, 0x22, 0x48, 0x82, 255);
  else
    SDL_SetRenderDrawColor(r, 0x0f, 0x34, 0x60, 255);
  SDL_RenderFillRect(r, &rect);
}

static void renderText(SDL_Renderer* r, TTF_Font* font, const char* text, int x, int y, Uint8 R, Uint8 G,
                       Uint8 B) {
  if (!font || !text) return;
  SDL_Color col = {R, G, B, 255};
  SDL_Surface* s = TTF_RenderUTF8_Blended(font, text, col);
  if (!s) return;
  SDL_Texture* tex = SDL_CreateTextureFromSurface(r, s);
  SDL_FreeSurface(s);
  if (!tex) return;
  int w, h;
  SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
  SDL_Rect dst = {x, y, w, h};
  SDL_RenderCopy(r, tex, nullptr, &dst);
  SDL_DestroyTexture(tex);
}
#endif

void render(App& app) {
  SDL_SetRenderDrawColor(app.renderer, 0x1a, 0x1a, 0x2e, 255);
  SDL_RenderClear(app.renderer);

  SDL_SetRenderDrawColor(app.renderer, 0x16, 0x21, 0x3e, 255);
  SDL_Rect topPalette = {PADDING, PADDING, BOARD_AREA_W - PADDING * 2, PALETTE_HEIGHT};
  SDL_Rect bottomPalette = {PADDING, PADDING + PALETTE_HEIGHT + PADDING + BOARD_SIZE + PADDING, BOARD_AREA_W - PADDING * 2,
                            PALETTE_HEIGHT};
  SDL_RenderFillRect(app.renderer, &topPalette);
  SDL_RenderFillRect(app.renderer, &bottomPalette);

  int topColor = app.boardFlipped ? 0 : 1;
  int bottomColor = app.boardFlipped ? 1 : 0;
  int palX = (BOARD_AREA_W - 6 * 48) / 2 + PADDING;
  int pieceSize = 44;

  for (int i = 0; i < 6; i++) {
    if (app.pieceTex[topColor][i])
      drawPiece(app.renderer, app.pieceTex[topColor][i], palX + i * 48, PADDING + 6, pieceSize);
    if (app.pieceTex[bottomColor][i])
      drawPiece(app.renderer, app.pieceTex[bottomColor][i], palX + i * 48,
                PADDING + PALETTE_HEIGHT + PADDING + BOARD_SIZE + PADDING + 6, pieceSize);
  }

  int boardY = PADDING + PALETTE_HEIGHT + PADDING;
  for (int row = 0; row < 8; row++) {
    int logicalRow = app.boardFlipped ? (7 - row) : row;
    for (int col = 0; col < 8; col++) {
      int sq = logicalRow * 8 + col;
      Uint32 c = ((logicalRow + col) % 2 == 0) ? COLOR_BOARD_LIGHT : COLOR_BOARD_DARK;
      bool isLegal = false;
      for (const auto& ls : app.legalMoveSquares)
        if (ls.index() == sq) {
          isLegal = true;
          break;
        }
      if (app.selectedSquare == sq)
        c = COLOR_HIGHLIGHT;
      else if (isLegal)
        c = COLOR_LEGAL;
      SDL_SetRenderDrawColor(app.renderer, (c >> 16) & 0xff, (c >> 8) & 0xff, c & 0xff, 0xff);
      SDL_Rect rect = {PADDING + col * SQUARE_SIZE, boardY + row * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
      SDL_RenderFillRect(app.renderer, &rect);
      if (isLegal && app.selectedSquare != sq) {
        int cx = rect.x + SQUARE_SIZE / 2, cy = rect.y + SQUARE_SIZE / 2;
        int r = SQUARE_SIZE / 6;
        SDL_SetRenderDrawColor(app.renderer, 90, 155, 90, 180);
        SDL_Rect dot = {cx - r, cy - r, r * 2, r * 2};
        SDL_RenderFillRect(app.renderer, &dot);
      }

      Piece p = app.pieces[sq];
      if (p != Piece::NONE && sq != app.dragFrom) {
        int color = p.color() == Color::WHITE ? 0 : 1;
        static const int PT_TO_TEX[] = {5, 4, 3, 2, 1, 0};
        int ti = PT_TO_TEX[static_cast<int>(p.type())];
        if (app.pieceTex[color][ti])
          drawPiece(app.renderer, app.pieceTex[color][ti], rect.x + 6, rect.y + 6, SQUARE_SIZE - 12);
      }
    }
  }

  if (app.dragFrom >= 0 && app.dragFrom < 64) {
    Piece p = app.pieces[app.dragFrom];
    if (p != Piece::NONE) {
      int color = p.color() == Color::WHITE ? 0 : 1;
      static const int PT_TO_TEX[] = {5, 4, 3, 2, 1, 0};
      int ti = PT_TO_TEX[static_cast<int>(p.type())];
      if (app.pieceTex[color][ti])
        drawPiece(app.renderer, app.pieceTex[color][ti], app.mouseX - SQUARE_SIZE / 2, app.mouseY - SQUARE_SIZE / 2,
                  SQUARE_SIZE);
    }
  } else if (app.dragFromPalette >= 0) {
    int color = app.dragFromPalette < 6 ? 0 : 1;
    int ti = app.dragFromPalette % 6;
    if (app.pieceTex[color][ti])
      drawPiece(app.renderer, app.pieceTex[color][ti], app.mouseX - SQUARE_SIZE / 2, app.mouseY - SQUARE_SIZE / 2,
                SQUARE_SIZE);
  }

  if (app.showSecondBestArrow && app.secondBestMoveFrom >= 0 && app.secondBestMoveTo >= 0) {
    int x1, y1, x2, y2;
    squareToPixel(app, boardY, app.secondBestMoveFrom, x1, y1);
    squareToPixel(app, boardY, app.secondBestMoveTo, x2, y2);
    drawArrowTint(app.renderer, x1, y1, x2, y2, 255, 210, 70);
  }
  if (app.showBestMoveArrow && app.bestMoveFrom >= 0 && app.bestMoveTo >= 0) {
    int x1, y1, x2, y2;
    squareToPixel(app, boardY, app.bestMoveFrom, x1, y1);
    squareToPixel(app, boardY, app.bestMoveTo, x2, y2);
    drawArrowTint(app.renderer, x1, y1, x2, y2, 70, 220, 120);
  }

  int btnY = PADDING + PALETTE_HEIGHT + PADDING + BOARD_SIZE + PALETTE_HEIGHT + PADDING * 2;
  SDL_SetRenderDrawColor(app.renderer, 0x0f, 0x34, 0x60, 255);
  SDL_Rect btnReset = {PADDING, btnY, 120, BUTTON_HEIGHT};
  SDL_RenderFillRect(app.renderer, &btnReset);

#if HAVE_TTF
  if (app.font)
    renderText(app.renderer, app.font, "Reset board", btnReset.x + 12, btnReset.y + 8, 234, 234, 234);
#endif

  int sideX = BOARD_AREA_W + PADDING;
  SDL_SetRenderDrawColor(app.renderer, 0x16, 0x21, 0x3e, 255);
  SDL_Rect sidebar = {sideX, PADDING, SIDEBAR_WIDTH - PADDING * 2, WINDOW_H - PADDING * 2};
  SDL_RenderFillRect(app.renderer, &sidebar);

#if HAVE_TTF
  if (app.font) {
    int ly = PADDING + 8;

    renderText(app.renderer, app.font, "To move:", sideX + 8, ly, 160, 160, 160);
    ly += 24;
    /* Left = Black (btn 1), right = White (btn 2). */
    SDL_Rect btnBlack = {sideX + 8, ly, 130, BUTTON_HEIGHT};
    SDL_Rect btnWhite = {sideX + 146, ly, 130, BUTTON_HEIGHT};
    bool whiteActive = app.board.sideToMove() == Color::WHITE;
    fillTurnButton(app.renderer, btnBlack, !whiteActive, app.hoverSidebarButton == 1);
    fillTurnButton(app.renderer, btnWhite, whiteActive, app.hoverSidebarButton == 2);
    renderText(app.renderer, app.font, "Black", sideX + 45, ly + 8, 234, 234, 234);
    renderText(app.renderer, app.font, "White", sideX + 183, ly + 8, 234, 234, 234);
    ly += BUTTON_HEIGHT + 20;

    bool hoverCalc = app.hoverSidebarButton == 3;
    SDL_SetRenderDrawColor(app.renderer, hoverCalc ? 0x24 : 0x1a, hoverCalc ? 0x7d : 0x6b, hoverCalc ? 0x58 : 0x4a, 255);
    SDL_Rect btnCalc = {sideX + 8, ly, SIDEBAR_WIDTH - PADDING * 2 - 16, BUTTON_HEIGHT};
    SDL_RenderFillRect(app.renderer, &btnCalc);
    renderText(app.renderer, app.font, "Calculate best move", sideX + 20, ly + 8, 255, 255, 255);
    ly += BUTTON_HEIGHT + 12;

    renderText(app.renderer, app.font, "Best move:", sideX + 8, ly, 160, 160, 160);
    ly += 20;
    if (!app.bestMoveSan.empty()) {
      renderText(app.renderer, app.fontSmall, app.bestMoveSan.c_str(), sideX + 8, ly, 100, 255, 150);
      ly += 16;
      if (!app.bestMoveEnglish.empty()) {
        std::string eng = app.bestMoveEnglish;
        if (eng.size() > 38) eng = eng.substr(0, 35) + "...";
        renderText(app.renderer, app.fontSmall, eng.c_str(), sideX + 8, ly, 200, 200, 200);
        ly += 16;
      }
      if (!app.openingLookupCompare.empty()) {
        std::string cmp = app.openingLookupCompare;
        if (cmp.size() > 42) cmp = cmp.substr(0, 39) + "...";
        renderText(app.renderer, app.fontSmall, cmp.c_str(), sideX + 8, ly, 180, 200, 255);
        ly += 16;
      }
      if (!app.openingTrieLine.empty()) {
        std::string t = app.openingTrieLine;
        if (t.size() > 52) t = t.substr(0, 49) + "...";
        renderText(app.renderer, app.fontSmall, t.c_str(), sideX + 8, ly, 90, 255, 140);
        ly += 16;
      }
      if (!app.openingTrieSecondLine.empty()) {
        std::string t = app.openingTrieSecondLine;
        if (t.size() > 52) t = t.substr(0, 49) + "...";
        renderText(app.renderer, app.fontSmall, t.c_str(), sideX + 8, ly, 255, 220, 100);
        ly += 16;
      }
      if (!app.openingHashLine.empty()) {
        std::string h = app.openingHashLine;
        if (h.size() > 52) h = h.substr(0, 49) + "...";
        renderText(app.renderer, app.fontSmall, h.c_str(), sideX + 8, ly, 120, 255, 200);
        ly += 16;
      }
      if (!app.openingHashSecondLine.empty()) {
        std::string h = app.openingHashSecondLine;
        if (h.size() > 52) h = h.substr(0, 49) + "...";
        renderText(app.renderer, app.fontSmall, h.c_str(), sideX + 8, ly, 255, 230, 120);
        ly += 16;
      }
      if (!app.openingScanLine.empty()) {
        std::string s = app.openingScanLine;
        if (s.size() > 52) s = s.substr(0, 49) + "...";
        renderText(app.renderer, app.fontSmall, s.c_str(), sideX + 8, ly, 170, 210, 255);
        ly += 16;
      }
      if (!app.moveGradeLine.empty()) {
        std::string gr = app.moveGradeLine;
        if (gr.size() > 42) gr = gr.substr(0, 39) + "...";
        renderText(app.renderer, app.fontSmall, gr.c_str(), sideX + 8, ly, 220, 200, 140);
        ly += 16;
      }
    } else {
      renderText(app.renderer, app.fontSmall, "Click button above", sideX + 8, ly, 160, 160, 160);
      ly += 16;
    }
    ly += 12;

    renderText(app.renderer, app.font, "Possible moves:", sideX + 8, ly, 160, 160, 160);
    ly += 20;
    if (app.legalMoveSquares.empty()) {
      renderText(app.renderer, app.fontSmall, "Click a piece", sideX + 8, ly, 160, 160, 160);
    } else {
      Movelist moves;
      getMovesForPiece(app.board, app.selectedSquare, moves);
      Board displayBoard = getBoardForPieceMoves(app.board, app.selectedSquare);
      int count = 0;
      for (size_t i = 0; i < moves.size() && count < 8; i++) {
        if (moves[i].from() == Square(app.selectedSquare)) {
          std::string san = uci::moveToSan(displayBoard, moves[i]);
          renderText(app.renderer, app.fontSmall, san.c_str(), sideX + 8, ly, 234, 234, 234);
          std::string eng = moveToPlainEnglish(displayBoard, moves[i]);
          if (eng.size() > 36) eng = eng.substr(0, 33) + "...";
          renderText(app.renderer, app.fontSmall, eng.c_str(), sideX + 8, ly + 14, 180, 180, 180);
          ly += 32;
          count++;
        }
      }
    }
    ly += 16;

    renderText(app.renderer, app.font, "FEN:", sideX + 8, ly, 160, 160, 160);
    std::string fen = app.board.getFen();
    if (fen.size() > 45) fen = fen.substr(0, 42) + "...";
    renderText(app.renderer, app.fontSmall, fen.c_str(), sideX + 8, ly + 16, 200, 200, 200);
  }
#endif

  SDL_RenderPresent(app.renderer);
}
