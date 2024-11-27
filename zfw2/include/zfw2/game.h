#pragma once

#include <GLFW/glfw3.h>
#include <AL/alc.h>
#include <zfw2/scenes.h>

namespace zfw2
{

constexpr int gk_glVersionMajor = 4;
constexpr int gk_glVersionMinor = 1;

constexpr int gk_targTicksPerSec = 60;
constexpr double gk_targTickDur = 1.0 / gk_targTicksPerSec;

void run_game();

}
