#include "sdl_helpers.hpp"
#include <SDL2/SDL.h>
#include <cstdlib>
#include <cstring>

using namespace std;
void sdlApplyHintsBeforeInit() {
#ifdef SDL_HINT_WINDOWS_DPI_AWARENESS
  SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");
#endif
  /* Partner workaround: 1:1 framebuffer if Retina / fractional scaling still looks wrong:
   *   CHESS_CALC_DISABLE_HIGHDPI=1 ./chess-calc   (Windows: set CHESS_CALC_DISABLE_HIGHDPI=1) */
  const char* noHi = getenv("CHESS_CALC_DISABLE_HIGHDPI");
  if (noHi && strcmp(noHi, "1") == 0)
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "1");
  else
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
  /* Nearest scaling pairs well with integer scale (crisp board); linear is fallback. */
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
}

void sdlSetupRendererLogical(SDL_Renderer* renderer, int logicalW, int logicalH) {
  if (!renderer || logicalW <= 0 || logicalH <= 0) return;
  if (SDL_RenderSetLogicalSize(renderer, logicalW, logicalH) != 0)
    SDL_Log("SDL_RenderSetLogicalSize failed: %s", SDL_GetError());
  /* Uniform scale (2x, 3x…) on HiDPI; avoids stretched / uneven scaling on some Macs. */
  if (SDL_RenderSetIntegerScale(renderer, SDL_TRUE) != 0)
    SDL_Log("SDL_RenderSetIntegerScale failed: %s", SDL_GetError());
}
