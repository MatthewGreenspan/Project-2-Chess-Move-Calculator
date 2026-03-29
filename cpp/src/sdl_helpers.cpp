#include "sdl_helpers.hpp"
#include <SDL2/SDL.h>
#include <cstdlib>
#include <cstring>

using namespace std;
// sets sdl hints before any window or renderer is made
void sdlApplyHintsBeforeInit() {
#ifdef SDL_HINT_WINDOWS_DPI_AWARENESS
  SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");
#endif
  const char* noHi = getenv("CHESS_CALC_DISABLE_HIGHDPI");
  if (noHi && strcmp(noHi, "1") == 0)
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "1");
  else
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
}

// locks rendering to the logical ui size we want
void sdlSetupRendererLogical(SDL_Renderer* renderer, int logicalW, int logicalH) {
  if (!renderer || logicalW <= 0 || logicalH <= 0) return;
  if (SDL_RenderSetLogicalSize(renderer, logicalW, logicalH) != 0)
    SDL_Log("SDL_RenderSetLogicalSize failed: %s", SDL_GetError());
  if (SDL_RenderSetIntegerScale(renderer, SDL_TRUE) != 0)
    SDL_Log("SDL_RenderSetIntegerScale failed: %s", SDL_GetError());
}
