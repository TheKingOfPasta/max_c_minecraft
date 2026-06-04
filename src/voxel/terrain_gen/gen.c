#include "gen.h"

#include "utils/vec3.h"
#include "voxel/block.h"

void fill_chunk(Chunk* c, BlockType b, vec3_u8 min, vec3_u8 max)
{
    for (u8 x = min.x; x < max.x; x++)
        for (u8 y = min.y; y < max.y; y++)
            for (u8 z = min.z; z < max.z; z++)
            {
                c->blocks[x + CHUNK_SIZE * (y + CHUNK_SIZE * z)] = ((Block){ .type = b });
            }
}

void gen_terrain([[maybe_unused]] i64 seed, Chunk* c)
{
    if (c->pos.y < 0)
    {
        fill_chunk(c, BLK_DIRT, (vec3_u8){ 0, 0, 0 },
                   (vec3_u8){ CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE });
    }
}
