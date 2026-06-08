#pragma once

#include "voxel/chunk.h"
#include "voxel/face.h"

void chunk_to_faces(Chunk* chunk, MAP(ChunkPos, ChunkPtr) * chunks, VECTOR(Face) * buf);
