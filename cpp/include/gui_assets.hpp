#pragma once

#include "app.hpp"
#include <string>

using namespace std;

// finds where the asset folder is at runtime
string findAssetPath();
// loads textures/fonts the gui needs
bool loadAssets(App& app);
