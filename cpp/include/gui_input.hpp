#pragma once

#include "app.hpp"

int getSquareAt(const App& app, int pixelX, int pixelY);
bool inBoard(const App& app, int x, int y);
int getPalettePieceAt(const App& app, int pixelX, int pixelY);
bool inPalette(int y);
int getSidebarButtonAt(int x, int y);
