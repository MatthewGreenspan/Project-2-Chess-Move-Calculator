/**
 * Chess Move Calculator - C++ Native Window
 * SDL2 GUI: puzzle setup, legal moves, Stockfish best move, palettes, sidebar.
 */
#include "chess_trie.h"
#include "pgn_parser.h"
#include "chess.hpp"
#include "stockfish.hpp"
#include <SDL2/SDL.h>
#include <array>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#if __has_include(<SDL2/SDL_ttf.h>)
#include <SDL2/SDL_ttf.h>
#define HAVE_TTF 1
#else
#define HAVE_TTF 0
#endif

using namespace chess;

// Layout - bigger window, wider sidebar
const int SQUARE_SIZE = 72;
const int BOARD_SIZE = 8 * SQUARE_SIZE;
const int PALETTE_HEIGHT = 56;
const int BUTTON_HEIGHT = 36;
const int PADDING = 16;
const int SIDEBAR_WIDTH = 300;
const int BOARD_AREA_W = BOARD_SIZE + PADDING * 2;
const int WINDOW_W = BOARD_AREA_W + SIDEBAR_WIDTH;
const int WINDOW_H = PALETTE_HEIGHT + PADDING + BOARD_SIZE + PALETTE_HEIGHT + PADDING + BUTTON_HEIGHT + PADDING * 2;

const Uint32 COLOR_BG = 0x1a1a2eff;
const Uint32 COLOR_SURFACE = 0x16213eff;
const Uint32 COLOR_BOARD_LIGHT = 0xf0d9b5ff;
const Uint32 COLOR_BOARD_DARK = 0xb58863ff;
const Uint32 COLOR_HIGHLIGHT = 0x7eb8da80;  /* soft blue - calm selection */
const Uint32 COLOR_LEGAL = 0x5a9b5a80;      /* muted green - legal squares */
const Uint32 COLOR_ACCENT = 0x0f3460ff;

const char* PIECE_NAMES[] = {"king", "queen", "rook", "bishop", "knight", "pawn"};
const char* PIECE_NAMES_CAP[] = {"King", "Queen", "Rook", "Bishop", "Knight", "Pawn"};

ChessTrie g_trie;

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

static void boardToArray(const Board& board, std::array<Piece, 64>& out) {
  for (int i = 0; i < 64; i++) out[i] = board.at<Piece>(Square(i));
}

static std::string boardToFen(const std::array<Piece, 64>& pieces, Color toMove) {
  std::string fen;
  for (int r = 7; r >= 0; r--) {
    int empty = 0;
    for (int f = 0; f < 8; f++) {
      Piece p = pieces[r * 8 + f];
      if (p == Piece::NONE) empty++;
      else {
        if (empty > 0) { fen += std::to_string(empty); empty = 0; }
        fen += static_cast<std::string>(p);
      }
    }
    if (empty > 0) fen += std::to_string(empty);
    if (r > 0) fen += '/';
  }
  fen += toMove == Color::WHITE ? " w" : " b";
  fen += " KQkq - 0 1";
  return fen;
}

static std::string moveToPlainEnglish(const Board& board, const Move& move) {
  if (move == Move::NO_MOVE) return "";
  PieceType pt = board.at<PieceType>(move.from());
  std::string toSq = static_cast<std::string>(move.to());
  bool capture = board.isCapture(move);

  if (move.typeOf() == Move::CASTLING) {
    return (move.to() > move.from()) ? "Kingside castling" : "Queenside castling";
  }
  if (move.typeOf() == Move::ENPASSANT) {
    return "Pawn captures en passant on " + toSq;
  }
  if (move.typeOf() == Move::PROMOTION) {
    std::string promo = PIECE_NAMES_CAP[static_cast<int>(move.promotionType())];
    return "Pawn promotes to " + promo + " on " + toSq;
  }

  std::string piece = PIECE_NAMES_CAP[static_cast<int>(pt)];
  if (capture)
    return piece + " captures on " + toSq;
  return piece + " to " + toSq;
}

static void clearBestMoveDisplay(App& app) {
  app.bestMoveFrom = -1;
  app.bestMoveTo = -1;
  app.showBestMoveArrow = false;
}

static bool hasValidKingCount(const Board& board) {
  auto wk = board.pieces(PieceType::KING, Color::WHITE);
  auto bk = board.pieces(PieceType::KING, Color::BLACK);
  return wk.count() == 1 && bk.count() == 1;
}
static void updateBestMove(App& app, bool force = false) {
    std::string fen = app.board.getFen();
    if (!force && fen == app.lastAnalyzedFen) return;
    app.lastAnalyzedFen = fen;
    app.bestMoveUci = "";
    app.bestMoveSan = "";
    app.bestMoveEnglish = "";
    app.bestMoveFrom = -1;
    app.bestMoveTo = -1;

    if (!hasValidKingCount(app.board)) {
        app.bestMoveEnglish = "Invalid position (need 1 king each)";
        return;
    }
    Movelist moves;
    movegen::legalmoves(moves, app.board);
    if (moves.size() == 0) {
        app.bestMoveEnglish = "Game over";
        return;
    }

    // -------------------------------------------------------
    // Priority 1: trie (most popular move from database)
    // Only works if moves were played in sequence this session
    // -------------------------------------------------------
    if (!app.movesPlayed.empty()) {
        string trieBest = g_trie.getBestMove(app.movesPlayed);
        if (!trieBest.empty()) {
            app.bestMoveSan = trieBest;
            app.bestMoveEnglish = "Most popular in your rating range";
            // Convert SAN back to from/to squares for the arrow
            for (size_t i = 0; i < moves.size(); i++) {
                if (uci::moveToSan(app.board, moves[i]) == trieBest) {
                    app.bestMoveFrom = moves[i].from().index();
                    app.bestMoveTo   = moves[i].to().index();
                    app.showBestMoveArrow = true;
                    break;
                }
            }
            return;
        }
    }

    // -------------------------------------------------------
    // Priority 2: Matt's FEN hashmap (manually set positions)
    // Swap in Matt's actual lookup function here
    // -------------------------------------------------------
    // string hashmapBest = matt_hashmap.getBestMove(fen);
    // if (!hashmapBest.empty()) { ... same pattern ... return; }

    // -------------------------------------------------------
    // Priority 3: Stockfish (existing logic, unchanged)
    // -------------------------------------------------------
    string uci = getBestMoveFromStockfish(fen, 400);
    Move m = Move::NO_MOVE;
    if (!uci.empty()) m = uci::uciToMove(app.board, uci);
    if (m == Move::NO_MOVE) m = moves[0];

    app.bestMoveUci     = uci;
    app.bestMoveSan     = uci::moveToSan(app.board, m);
    app.bestMoveEnglish = moveToPlainEnglish(app.board, m);
    app.bestMoveFrom    = m.from().index();
    app.bestMoveTo      = m.to().index();
    app.showBestMoveArrow = true;
}
// static void updateBestMove(App& app, bool force = false) {
//   std::string fen = app.board.getFen();
//   if (!force && fen == app.lastAnalyzedFen) return;
//   app.lastAnalyzedFen = fen;
//   app.bestMoveUci = "";
//   app.bestMoveSan = "";
//   app.bestMoveEnglish = "";
//   app.bestMoveFrom = -1;
//   app.bestMoveTo = -1;

//   if (!hasValidKingCount(app.board)) {
//     app.bestMoveEnglish = "Invalid position (need 1 king each)";
//     return;
//   }

//   Movelist moves;
//   movegen::legalmoves(moves, app.board);
//   if (moves.size() == 0) {
//     app.bestMoveEnglish = "Game over";
//     return;
//   }

//   std::string uci = getBestMoveFromStockfish(fen, 400);
//   Move m = Move::NO_MOVE;
//   if (!uci.empty()) {
//     m = uci::uciToMove(app.board, uci);
//   }
//   if (m == Move::NO_MOVE) {
//     m = moves[0];
//     app.bestMoveSan = static_cast<std::string>(m.from()) + "-" + static_cast<std::string>(m.to());
//     app.bestMoveEnglish = moveToPlainEnglish(app.board, m);
//   } else {
//     app.bestMoveUci = uci;
//     app.bestMoveSan = uci::moveToSan(app.board, m);
//     app.bestMoveEnglish = moveToPlainEnglish(app.board, m);
//   }
//   app.bestMoveFrom = m.from().index();
//   app.bestMoveTo = m.to().index();
//   app.showBestMoveArrow = true;
// }

static std::string findAssetPath() {
  char* base = SDL_GetBasePath();
  if (base) {
    std::string p(base);
    SDL_free(base);
    std::string tries[] = {p + "assets/", p + "../assets/", "./assets/", "assets/"};
    for (const auto& t : tries) {
      std::string f = t + "white/king.png";
      if (SDL_RWFromFile(f.c_str(), "r")) return t;
    }
  }
  return "assets/";
}

static SDL_Texture* loadTexture(SDL_Renderer* r, const std::string& path) {
  int w, h, n;
  unsigned char* data = stbi_load(path.c_str(), &w, &h, &n, 4);
  if (data) {
    SDL_Surface* s = SDL_CreateRGBSurfaceFrom(data, w, h, 32, w * 4, 0xff, 0xff00, 0xff0000, 0xff000000);
    SDL_Texture* t = nullptr;
    if (s) {
      t = SDL_CreateTextureFromSurface(r, s);
      SDL_FreeSurface(s);
    }
    stbi_image_free(data);
    if (t) return t;
  }
  SDL_Surface* s = SDL_CreateRGBSurface(0, 64, 64, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
  if (!s) return nullptr;
  SDL_FillRect(s, nullptr, SDL_MapRGBA(s->format, 200, 200, 200, 220));
  SDL_Texture* t = SDL_CreateTextureFromSurface(r, s);
  SDL_FreeSurface(s);
  return t;
}

static bool loadAssets(App& app) {
  const char* colors[] = {"white", "black"};
  for (int c = 0; c < 2; c++)
    for (int p = 0; p < 6; p++) {
      std::string path = app.assetPath + colors[c] + "/" + PIECE_NAMES[p] + ".png";
      app.pieceTex[c][p] = loadTexture(app.renderer, path);
      if (!app.pieceTex[c][p]) return false;
    }
#if HAVE_TTF
  const char* fontPaths[] = {
    "/System/Library/Fonts/Helvetica.ttc",
    "/System/Library/Fonts/SFNS.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    nullptr
  };
  for (int i = 0; fontPaths[i]; i++) {
    app.font = TTF_OpenFont(fontPaths[i], 18);
    if (app.font) {
      app.fontSmall = TTF_OpenFont(fontPaths[i], 12);
      if (!app.fontSmall) app.fontSmall = app.font;
      break;
    }
  }
#endif
  return true;
}

static int getSquareAt(App& app, int pixelX, int pixelY) {
  int py = pixelY - PADDING - PALETTE_HEIGHT - PADDING;
  if (py < 0 || py >= BOARD_SIZE) return -1;
  int px = pixelX - PADDING;
  if (px < 0 || px >= BOARD_SIZE) return -1;
  int col = px / SQUARE_SIZE;
  int row = py / SQUARE_SIZE;
  if (app.boardFlipped) row = 7 - row;
  return row * 8 + col;
}

static bool inBoard(App& app, int x, int y) {
  int py = y - PADDING - PALETTE_HEIGHT - PADDING;
  int px = x - PADDING;
  return px >= 0 && px < BOARD_SIZE && py >= 0 && py < BOARD_SIZE;
}

static int getPalettePieceAt(App& app, int pixelX, int pixelY) {
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

static bool inPalette(int y) {
  int py1 = y - PADDING;
  int py2 = y - PADDING - PALETTE_HEIGHT - PADDING - BOARD_SIZE - PADDING;
  return (py1 >= 0 && py1 < PALETTE_HEIGHT) || (py2 >= 0 && py2 < PALETTE_HEIGHT);
}

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

static void drawArrow(SDL_Renderer* r, int x1, int y1, int x2, int y2) {
  float dx = (float)(x2 - x1), dy = (float)(y2 - y1);
  float len = std::sqrt(dx * dx + dy * dy);
  if (len < 4.0f) return;
  dx /= len; dy /= len;

  const float shaftEnd = len - 26.0f;
  if (shaftEnd < 2.0f) return;

  const int thick = 7;
  const float arrLen = 26.0f;
  const float wing = 14.0f;
  float perpX = -dy, perpY = dx;

  auto drawThickLine = [r](int xa, int ya, int xb, int yb, int w) {
    float dx = (float)(xb - xa), dy = (float)(yb - ya);
    float l = std::sqrt(dx*dx + dy*dy);
    if (l < 0.5f) return;
    dx /= l; dy /= l;
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

  SDL_SetRenderDrawColor(r, 40, 30, 10, 255);
  drawThickLine(x1, y1, sx2, sy2, thick + 3);
  drawThickLine(x2, y2, ax, ay, 4);
  drawThickLine(x2, y2, bx, by, 4);
  drawThickLine(ax, ay, bx, by, 4);

  SDL_SetRenderDrawColor(r, 255, 220, 90, 255);
  drawThickLine(x1, y1, sx2, sy2, thick);
  SDL_RenderDrawLine(r, x2, y2, ax, ay);
  SDL_RenderDrawLine(r, x2, y2, bx, by);
  SDL_RenderDrawLine(r, ax, ay, bx, by);

  SDL_SetRenderDrawColor(r, 255, 235, 150, 255);
  drawThickLine(x1, y1, sx2, sy2, thick - 3);
}

#if HAVE_TTF
static void renderText(SDL_Renderer* r, TTF_Font* font, const char* text, int x, int y, Uint8 R, Uint8 G, Uint8 B) {
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

static int getSidebarButtonAt(int x, int y) {
  int sideX = BOARD_AREA_W + PADDING;
  if (x < sideX || x >= sideX + SIDEBAR_WIDTH - PADDING * 2) return -1;
  int btnY = PADDING + 32;
  if (y >= btnY && y < btnY + BUTTON_HEIGHT) {
    int bx = x - sideX - 8;
    if (bx >= 0 && bx < 130) return 1;   // White to move
    if (bx >= 138 && bx < 268) return 2; // Black to move
  }
  btnY = PADDING + 32 + BUTTON_HEIGHT + 20;  // Calculate best move button
  if (y >= btnY && y < btnY + BUTTON_HEIGHT) {
    if (x >= sideX + 8 && x < sideX + SIDEBAR_WIDTH - PADDING * 2 - 8) return 3; // Calculate best move
  }
  return -1;
}

// Get board with turn set so the piece at squareIndex is the side to move (for SAN/display)
static Board getBoardForPieceMoves(const Board& board, int squareIndex) {
  Piece p = board.at<Piece>(Square(squareIndex));
  if (p == Piece::NONE) return board;
  if (p.color() == board.sideToMove()) return board;
  Board b = board;
  std::string fen = board.getFen();
  size_t pos = fen.find(' ');
  if (pos != std::string::npos && pos + 2 <= fen.size())
    fen[pos + 1] = (fen[pos + 1] == 'w') ? 'b' : 'w';
  b.setFen(fen);
  return b;
}

// Get legal moves for piece at square (works for any piece, regardless of turn)
static void getMovesForPiece(const Board& board, int squareIndex, Movelist& out) {
  out.clear();
  if (!hasValidKingCount(board)) return;
  Piece p = board.at<Piece>(Square(squareIndex));
  if (p == Piece::NONE) return;
  Board b = getBoardForPieceMoves(board, squareIndex);
  movegen::legalmoves(out, b);
}

static void updateLegalMoves(App& app) {
  app.legalMoveSquares.clear();
  if (app.selectedSquare < 0) return;
  Movelist moves;
  getMovesForPiece(app.board, app.selectedSquare, moves);
  for (size_t i = 0; i < moves.size(); i++) {
    if (moves[i].from() == Square(app.selectedSquare))
      app.legalMoveSquares.push_back(moves[i].to());
  }
}

static void render(App& app) {
  SDL_SetRenderDrawColor(app.renderer, 0x1a, 0x1a, 0x2e, 255);
  SDL_RenderClear(app.renderer);

  // Palettes
  SDL_SetRenderDrawColor(app.renderer, 0x16, 0x21, 0x3e, 255);
  SDL_Rect topPalette = {PADDING, PADDING, BOARD_AREA_W - PADDING * 2, PALETTE_HEIGHT};
  SDL_Rect bottomPalette = {PADDING, PADDING + PALETTE_HEIGHT + PADDING + BOARD_SIZE + PADDING, BOARD_AREA_W - PADDING * 2, PALETTE_HEIGHT};
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
      drawPiece(app.renderer, app.pieceTex[bottomColor][i], palX + i * 48, PADDING + PALETTE_HEIGHT + PADDING + BOARD_SIZE + PADDING + 6, pieceSize);
  }

  // Board
  int boardY = PADDING + PALETTE_HEIGHT + PADDING;
  for (int row = 0; row < 8; row++) {
    int logicalRow = app.boardFlipped ? (7 - row) : row;
    for (int col = 0; col < 8; col++) {
      int sq = logicalRow * 8 + col;
      Uint32 c = ((logicalRow + col) % 2 == 0) ? COLOR_BOARD_LIGHT : COLOR_BOARD_DARK;
      bool isLegal = false;
      for (const auto& ls : app.legalMoveSquares)
        if (ls.index() == sq) { isLegal = true; break; }
      if (app.selectedSquare == sq) c = COLOR_HIGHLIGHT;
      else if (isLegal) c = COLOR_LEGAL;
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
        drawPiece(app.renderer, app.pieceTex[color][ti], app.mouseX - SQUARE_SIZE / 2, app.mouseY - SQUARE_SIZE / 2, SQUARE_SIZE);
    }
  } else if (app.dragFromPalette >= 0) {
    int color = app.dragFromPalette < 6 ? 0 : 1;
    int ti = app.dragFromPalette % 6;
    if (app.pieceTex[color][ti])
      drawPiece(app.renderer, app.pieceTex[color][ti], app.mouseX - SQUARE_SIZE / 2, app.mouseY - SQUARE_SIZE / 2, SQUARE_SIZE);
  }

  if (app.showBestMoveArrow && app.bestMoveFrom >= 0 && app.bestMoveTo >= 0) {
    int x1, y1, x2, y2;
    squareToPixel(app, boardY, app.bestMoveFrom, x1, y1);
    squareToPixel(app, boardY, app.bestMoveTo, x2, y2);
    drawArrow(app.renderer, x1, y1, x2, y2);
  }

  // Buttons (Reset only - White/Black to move are in sidebar)
  int btnY = PADDING + PALETTE_HEIGHT + PADDING + BOARD_SIZE + PALETTE_HEIGHT + PADDING * 2;
  SDL_SetRenderDrawColor(app.renderer, 0x0f, 0x34, 0x60, 255);
  SDL_Rect btnReset = {PADDING, btnY, 120, BUTTON_HEIGHT};
  SDL_RenderFillRect(app.renderer, &btnReset);

#if HAVE_TTF
  if (app.font)
    renderText(app.renderer, app.font, "Reset board", btnReset.x + 12, btnReset.y + 8, 234, 234, 234);
#endif

  // Sidebar
  int sideX = BOARD_AREA_W + PADDING;
  SDL_SetRenderDrawColor(app.renderer, 0x16, 0x21, 0x3e, 255);
  SDL_Rect sidebar = {sideX, PADDING, SIDEBAR_WIDTH - PADDING * 2, WINDOW_H - PADDING * 2};
  SDL_RenderFillRect(app.renderer, &sidebar);

#if HAVE_TTF
  if (app.font) {
    int ly = PADDING + 8;

    // White to move / Black to move buttons
    renderText(app.renderer, app.font, "To move:", sideX + 8, ly, 160, 160, 160);
    ly += 24;
    SDL_SetRenderDrawColor(app.renderer, 0x0f, 0x34, 0x60, 255);
    SDL_Rect btnWhite = {sideX + 8, ly, 130, BUTTON_HEIGHT};
    SDL_Rect btnBlack = {sideX + 146, ly, 130, BUTTON_HEIGHT};
    bool whiteActive = app.board.sideToMove() == Color::WHITE;
    if (whiteActive) SDL_SetRenderDrawColor(app.renderer, 0x2a, 0x5a, 0x90, 255);
    SDL_RenderFillRect(app.renderer, &btnWhite);
    SDL_SetRenderDrawColor(app.renderer, 0x0f, 0x34, 0x60, 255);
    if (!whiteActive) SDL_SetRenderDrawColor(app.renderer, 0x2a, 0x5a, 0x90, 255);
    SDL_RenderFillRect(app.renderer, &btnBlack);
    renderText(app.renderer, app.font, "White", sideX + 45, ly + 8, 234, 234, 234);
    renderText(app.renderer, app.font, "Black", sideX + 183, ly + 8, 234, 234, 234);
    ly += BUTTON_HEIGHT + 20;

    // Calculate best move button
    SDL_SetRenderDrawColor(app.renderer, 0x1a, 0x6b, 0x4a, 255);
    SDL_Rect btnCalc = {sideX + 8, ly, SIDEBAR_WIDTH - PADDING * 2 - 16, BUTTON_HEIGHT};
    SDL_RenderFillRect(app.renderer, &btnCalc);
    renderText(app.renderer, app.font, "Calculate best move", sideX + 20, ly + 8, 255, 255, 255);
    ly += BUTTON_HEIGHT + 12;

    // Best move (Stockfish)
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
    } else {
      renderText(app.renderer, app.fontSmall, "Click button above", sideX + 8, ly, 160, 160, 160);
      ly += 16;
    }
    ly += 12;

    // Possible moves (when piece selected)
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

static void doDrop(App& app, int toSquare) {
  if (app.dragFrom >= 0 && app.dragFrom < 64) {
    Piece p = app.pieces[app.dragFrom];
    if (p != Piece::NONE && toSquare >= 0 && toSquare < 64) {
      app.pieces[app.dragFrom] = Piece::NONE;
      app.pieces[toSquare] = p;
      std::string fen = boardToFen(app.pieces, app.board.sideToMove());
      app.board.setFen(fen);
      app.selectedSquare = -1;
      app.legalMoveSquares.clear();
    }
    app.dragFrom = -1;
  } else if (app.dragFromPalette >= 0 && toSquare >= 0 && toSquare < 64) {
    int color = app.dragFromPalette < 6 ? 0 : 1;
    static const PieceType PT_MAP[] = {PieceType::KING, PieceType::QUEEN, PieceType::ROOK, PieceType::BISHOP, PieceType::KNIGHT, PieceType::PAWN};
    PieceType pt = PT_MAP[app.dragFromPalette % 6];
    Piece p(PieceType(pt), color == 0 ? Color::WHITE : Color::BLACK);
    app.pieces[toSquare] = p;
    std::string fen = boardToFen(app.pieces, app.board.sideToMove());
    if (app.board.setFen(fen)) app.selectedSquare = -1;
    app.dragFromPalette = -1;
  }
  updateLegalMoves(app);
  clearBestMoveDisplay(app);
}

static void doRemove(App& app) {
  if (app.dragFrom >= 0 && app.dragFrom < 64) {
    app.pieces[app.dragFrom] = Piece::NONE;
    std::string fen = boardToFen(app.pieces, app.board.sideToMove());
    app.board.setFen(fen);
    app.dragFrom = -1;
    app.selectedSquare = -1;
    app.legalMoveSquares.clear();
    clearBestMoveDisplay(app);
  }
}

static bool tryLegalMove(App& app, int fromSq, int toSq) {
  Movelist moves;
  getMovesForPiece(app.board, fromSq, moves);
  for (size_t i = 0; i < moves.size(); i++) {
    if (moves[i].from() == Square(fromSq) && moves[i].to() == Square(toSq)) {
      Board b = getBoardForPieceMoves(app.board, fromSq);

      app.movesPlayed.push_back(uci::moveToSan(b, moves[i]));

      b.makeMove(moves[i]);
      app.board = b;
      boardToArray(app.board, app.pieces);
      app.selectedSquare = -1;
      app.legalMoveSquares.clear();
      clearBestMoveDisplay(app);
      return true;
    }
  }
  return false;
}

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;
  
  cout << "Loading opening database...\n";
    PGNParser parser(1200, 1600, 20);
    parser.parse("lichess_games.pgn", [&](const GameData& game) {
        g_trie.insertGame(game.moves);
    });
    g_trie.prune(5);
    cout << "Database ready.\n";

  SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
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

  app.window = SDL_CreateWindow("Chess Move Calculator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
  if (!app.window) {
    SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
#if HAVE_TTF
    TTF_Quit();
#endif
    SDL_Quit();
    return 1;
  }

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
            if (tryLegalMove(app, app.selectedSquare, sq)) { /* done */ }
            else app.selectedSquare = -1, app.legalMoveSquares.clear();
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
              if (tryLegalMove(app, app.dragFrom, sq)) { }
              else doDrop(app, sq);
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
