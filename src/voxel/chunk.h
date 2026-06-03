#pragma once

#include <stdint.h>

#include "block.h"

#define CHUNK_SIZE 16

typedef struct
{
    int64_t x;
    int64_t y;

    Block blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
} Chunk;

Chunk create_random_chunk(int64_t x, int64_t y);
