#pragma once

#include "app.hpp"

using namespace std;
// handles piece drop in sandbox mode
void doDrop(App& app, int toSquare);
void doRemove(App& app);
// tries a real legal move before sandbox fallback
bool tryLegalMove(App& app, int fromSq, int toSq);
