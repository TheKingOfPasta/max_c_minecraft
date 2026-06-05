#pragma once

#include "utils/type.h"
#include "utils/vec3.h"

float noise2d(i64 seed, VEC3(float) pos, float scale, float amplitude);
float noise3d(i64 seed, VEC3(float) pos, float scale, float amplitude);
