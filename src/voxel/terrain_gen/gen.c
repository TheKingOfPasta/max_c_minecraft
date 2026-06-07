#include "gen.h"

#include <math.h>

#include "noise.h"
#include "opengl/tracy.h"
#include "utils/utils.h"
#include "utils/vec3.h"
#include "voxel/block.h"
#include "voxel/chunk.h"

static inline BlockType gen_block(i64 seed, VEC3(float) lpos)
{
    float density = noise3d(seed, lpos, 64.0f, 1.0f);
    return density > 0.8f ? BLK_DIRT : BLK_AIR;
}

void gen_terrain(i64 seed, Chunk* c)
{
    TracyZone(ctx, "gen_terrain");
    for (u8 x = 0; x < CHUNK_SIZE; x++)
        for (u8 z = 0; z < CHUNK_SIZE; z++)
            for (u8 y = 0; y < CHUNK_SIZE; y++)
            {
                c->blocks[x + CHUNK_SIZE * (y + CHUNK_SIZE * z)] = (Block){
                    .type = gen_block(
                        seed, VEC3_CAST(float, chunk_local_to_world(c->pos, (VEC3(u8)){ x, y, z })))
                };
            }
    TracyZoneEnd(ctx);
}
