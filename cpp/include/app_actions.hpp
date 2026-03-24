#pragma once

#include "app.hpp"

void doDrop(App& app, int toSquare);
void doRemove(App& app);
bool tryLegalMove(App& app, int fromSq, int toSq);
