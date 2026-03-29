#include "gui_input.hpp"
#include "gui_constants.hpp"

using namespace std;
// turns pixel coords into the real board index
int getSquareAt(const App& app, int pixelX, int pixelY) {
  int py = pixelY - PADDING - PALETTE_HEIGHT - PADDING;
  if (py < 0 || py >= BOARD_SIZE) return -1;
  int px = pixelX - PADDING;
  if (px < 0 || px >= BOARD_SIZE) return -1;
  int col = px / SQUARE_SIZE;
  int row = py / SQUARE_SIZE;
  if (!app.boardFlipped) row = 7 - row;
  return row * 8 + col;
}

// just checks if the mouse is over the board area
bool inBoard(const App& app, int x, int y) {
  int py = y - PADDING - PALETTE_HEIGHT - PADDING;
  int px = x - PADDING;
  return px >= 0 && px < BOARD_SIZE && py >= 0 && py < BOARD_SIZE;
}

// figures out which spare piece in the palette got clicked
int getPalettePieceAt(const App& app, int pixelX, int pixelY) {
  int py = pixelY - PADDING;
  if (py >= 0 && py < PALETTE_HEIGHT) {
    int cx = (BOARD_AREA_W - 6 * 48) / 2 + PADDING;
    int px = pixelX - cx;
    if (px >= 0 && px < 6 * 48) return (app.boardFlipped ? 0 : 6) + px / 48;
  }
  py = pixelY - PADDING - PALETTE_HEIGHT - PADDING - BOARD_SIZE - PADDING;
  if (py >= 0 && py < PALETTE_HEIGHT) {
    int cx = (BOARD_AREA_W - 6 * 48) / 2 + PADDING;
    int px = pixelX - cx;
    if (px >= 0 && px < 6 * 48) return (app.boardFlipped ? 6 : 0) + px / 48;
  }
  return -1;
}

// returns which sidebar button the mouse is on
int getSidebarButtonAt(int x, int y) {
  int sideX = BOARD_AREA_W + PADDING;
  if (x < sideX || x >= sideX + SIDEBAR_WIDTH - PADDING * 2) return -1;
  int btnY = PADDING + 32;
  if (y >= btnY && y < btnY + BUTTON_HEIGHT) {
    int bx = x - sideX - 8;
    if (bx >= 0 && bx < 130) return 1;
    if (bx >= 138 && bx < 268) return 2;
  }
  btnY = PADDING + 32 + BUTTON_HEIGHT + 20;
  if (y >= btnY && y < btnY + BUTTON_HEIGHT) {
    if (x >= sideX + 8 && x < sideX + SIDEBAR_WIDTH - PADDING * 2 - 8) return 3;
  }
  return -1;
}
