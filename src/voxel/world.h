#pragma once

#include "utils/container.h"
#include "voxel/chunk.h"

typedef struct
{
    MAP(ChunkPos, ChunkPtr) chunks;
} World;

World init_world(void);
