#pragma once
#include "utils/container.h"
#include "utils/type.h"
#include "utils/vec3.h"
#include "voxel/chunk.h"

// Each chunk gathers the tree blocks it should contain by reading trunk
// positions from neighbouring chunks — it never pushes into others.
void gen_trees(i32 seed, Chunk* c, MAP(ChunkPos, ChunkPtr)* world);
