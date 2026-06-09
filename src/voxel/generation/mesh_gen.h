#pragma once

#include "voxel/chunk.h"
#include "voxel/face.h"

typedef struct
{
    VEC3(i32) pos;
    int dir;
    VEC3(i32) widths;
    BlockType block_type;
} GreedyFace;

VECTOR_DECLARE(GreedyFace);

void chunk_to_faces(Chunk* chunk, MAP(ChunkPos, ChunkPtr) * chunks, VECTOR(Face) * buf);
