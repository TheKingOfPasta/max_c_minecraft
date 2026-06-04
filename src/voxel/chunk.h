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

    Block blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
} Chunk;

VECTOR_DECLARE(Face);

Chunk create_random_chunk(i64 x, i64 y);
VECTOR(Face) recreate_vertices(const Chunk* chunk);
const Block* chunk_get(const Chunk* chunk, VEC3(u8) pos);
