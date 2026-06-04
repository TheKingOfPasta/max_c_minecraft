#pragma once

#include <stddef.h>
#include <stdint.h>

#include <utils/container.h>
#include <utils/vec3.h>

#include "face.h"
#include "block.h"

#define CHUNK_SIZE 16
#define INSTANCE_MAX (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 6)

typedef struct
{
    VEC3(i64) pos;

    bool visited;

    Block blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
} Chunk;

typedef VEC3(i64) ChunkPos;
typedef Chunk* ChunkPtr;

static inline size_t chunkpos_hash(ChunkPos p)
{
    uint64_t h = (uint64_t)(uint32_t)p.x * 73856093u
        ^ (uint64_t)(uint32_t)p.y * 19349663u
        ^ (uint64_t)(uint32_t)p.z * 83492791u;
    return (size_t)h;
}

static inline bool chunkpos_eq(ChunkPos a, ChunkPos b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

MAP_DECLARE(ChunkPos, ChunkPtr, chunkpos_hash, chunkpos_eq);

VECTOR_DECLARE(Face);

Chunk* create_empty_chunk(VEC3(i64) pos);
Chunk* create_random_chunk(i64 x, i64 y, i64 z);
VECTOR(Face) chunk_to_faces(const Chunk* chunk, VECTOR(Face) face_instances);
const Block* chunk_get(const Chunk* chunk, VEC3(u8) pos);
