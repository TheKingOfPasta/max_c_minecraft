#include "chunk.h"

#include <stdlib.h>

#include "block.h"
#include "utils/vec3.h"

Chunk* create_empty_chunk(VEC3(i32) pos)
{
    Chunk* c = calloc(1, sizeof(Chunk));
    c->pos = pos;

    return c;
}

const Block* chunk_get(const Chunk* chunk, VEC3(u8) pos)
{
    return chunk->blocks + CHUNK_IDX(pos.x, pos.y, pos.z);
}
