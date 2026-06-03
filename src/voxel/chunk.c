#include "chunk.h"

#include <stdlib.h>

#include "block.h"

Chunk create_random_chunk(int64_t x, int64_t y)
{
    Chunk c = { .x = x,
                .y = y};

    for (uint8_t i = 0; i < CHUNK_SIZE; i++)
    for (uint8_t j = 0; j < CHUNK_SIZE; j++)
    for (uint8_t k = 0; k < CHUNK_SIZE; k++)
    {
        c.blocks[i * CHUNK_SIZE * CHUNK_SIZE + j * CHUNK_SIZE + k] = (Block){ .type = rand() % BLOCK_COUNT };
    }

    return c;
}
