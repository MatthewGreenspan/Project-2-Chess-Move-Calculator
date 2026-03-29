#pragma once

#include "app.hpp"

using namespace std;
// clears the sidebar move info
void clearBestMoveDisplay(App& app);
// recomputes best move stuff for the current board
void updateBestMove(App& app, bool force = false);
