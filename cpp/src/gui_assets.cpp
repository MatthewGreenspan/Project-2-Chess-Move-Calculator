#include "gui_assets.hpp"
#include "gui_constants.hpp"
#include <SDL2/SDL.h>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::string findAssetPath() {
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

bool loadAssets(App& app) {
  const char* colors[] = {"white", "black"};
  for (int c = 0; c < 2; c++)
    for (int p = 0; p < 6; p++) {
      std::string path = app.assetPath + colors[c] + "/" + PIECE_NAMES[p] + ".png";
      app.pieceTex[c][p] = loadTexture(app.renderer, path);
      if (!app.pieceTex[c][p]) return false;
    }
#if HAVE_TTF
  const char* fontPaths[] = {
#if defined(_WIN32)
      "C:\\Windows\\Fonts\\segoeui.ttf",
      "C:\\Windows\\Fonts\\arial.ttf",
#elif defined(__APPLE__)
      "/System/Library/Fonts/Helvetica.ttc",
      "/System/Library/Fonts/SFNS.ttf",
#else
      "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
      "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
#endif
      nullptr};
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
