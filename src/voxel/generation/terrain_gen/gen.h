#pragma once

#include "utils/type.h"
#include "voxel/chunk.h"

#define BORDER_UP 200
#define BORDER_DOWN -200
#define BORDER_SMOOTH 50

void gen_terrain(i32 seed, Chunk* c);
