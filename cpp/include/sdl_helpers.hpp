#pragma once

#include <SDL2/SDL.h>

/** Call before SDL_Init — DPI, HiDPI, render quality. */
void sdlApplyHintsBeforeInit();

/** After SDL_CreateRenderer — fixed logical size so layout matches all DPIs / platforms. */
void sdlSetupRendererLogical(SDL_Renderer* renderer, int logicalW, int logicalH);
