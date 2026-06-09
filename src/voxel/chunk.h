#pragma once

#include <stddef.h>
#include <utils/container.h>
#include <utils/vec3.h>

#include "block.h"

#define CHUNK_SIZE 32
#define CHUNK_SHIFT 5

typedef struct
{
    VEC3(i32) pos;

    bool meshed;

    size_t face_start_index;
    size_t face_count;

    Block blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
} Chunk;

typedef VEC3(i32) ChunkPos;
typedef Chunk* ChunkPtr;

static inline u32 chunkpos_hash(ChunkPos p)
{
    return (u32)p.x * 73856093u ^ (u32)p.y * 19349663u ^ (u32)p.z * 83492791u;
}

static inline bool chunkpos_eq(ChunkPos a, ChunkPos b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

MAP_DECLARE(ChunkPos, ChunkPtr, chunkpos_hash, chunkpos_eq);
VECTOR_DECLARE(ChunkPtr);

static inline VEC3(i32) chunk_local_to_world(VEC3(i32) chunk_pos, VEC3(u8) local)
{
    return (VEC3(i32)){
        .x = chunk_pos.x * CHUNK_SIZE + local.x,
        .y = chunk_pos.y * CHUNK_SIZE + local.y,
        .z = chunk_pos.z * CHUNK_SIZE + local.z,
    };
}

#define CHUNK_IDX(x, y, z) ((z) * CHUNK_SIZE * CHUNK_SIZE + (y) * CHUNK_SIZE + (x))
#define CHUNK_IDX_2D(x, y) ((y) * CHUNK_SIZE + (x))

Chunk* create_empty_chunk(VEC3(i32) pos);
const Block* chunk_get(const Chunk* chunk, VEC3(u8) pos);
