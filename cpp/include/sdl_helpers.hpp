#pragma once

#include <SDL2/SDL.h>

using namespace std;

// sets some sdl hints before window setup
void sdlApplyHintsBeforeInit();

void sdlSetupRendererLogical(SDL_Renderer* renderer, int logicalW, int logicalH);
