#pragma once

#include "app.hpp"

using namespace std;
// turns mouse coords into board square index
int getSquareAt(const App& app, int pixelX, int pixelY);
bool inBoard(const App& app, int x, int y);
int getPalettePieceAt(const App& app, int pixelX, int pixelY);
int getSidebarButtonAt(int x, int y);
