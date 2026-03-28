#pragma once

#include "app.hpp"

using namespace std;
int getSquareAt(const App& app, int pixelX, int pixelY);
bool inBoard(const App& app, int x, int y);
int getPalettePieceAt(const App& app, int pixelX, int pixelY);
int getSidebarButtonAt(int x, int y);
