#pragma once

#include "app.hpp"

using namespace std;
void doDrop(App& app, int toSquare);
void doRemove(App& app);
bool tryLegalMove(App& app, int fromSq, int toSq);
