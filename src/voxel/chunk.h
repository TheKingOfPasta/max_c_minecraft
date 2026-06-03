#pragma once

#include <stdint.h>

#include "block.h"

#define CHUNK_SIZE 16

typedef struct
{
    int64_t x;
    int64_t y;

    block blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
} chunk;

chunk create_random_chunk(int64_t x, int64_t y);
