#include "gen.h"

#include <math.h>

#include "utils/utils.h"
#include "utils/vec3.h"
#include "voxel/block.h"
#include "voxel/chunk.h"

static u32 hashXZ(i64 x, i64 z, i64 seed)
{
    u64 h = (u64)(x * 1619LL + z * 31337LL + seed * 6971LL);
    h ^= h >> 17;
    h *= 0xbf58476d1ce4e5b9ull;
    h ^= h >> 31;
    return (u32)h;
}

static float grid_val(i64 x, i64 z, i64 seed)
{
    return (float)(hashXZ(x, z, seed) >> 17) / (float)(1 << 15);
}

static float noise2d(i64 seed, VEC3(float) pos, float scale, float amplitude)
{
    VEC3_DIV_INPLACE(pos, scale);
    VEC3(i64) i = { (i64)floorf(pos.x), (i64)floorf(pos.y), (i64)floorf(pos.z) };
    float fx = pos.x - (float)i.x;
    float fz = pos.z - (float)i.z;
    return amplitude
        * SMOOTHLERP(SMOOTHLERP(grid_val(i.x, i.z, seed), grid_val(i.x + 1, i.z, seed), fx),
                     SMOOTHLERP(grid_val(i.x, i.z + 1, seed), grid_val(i.x + 1, i.z + 1, seed), fx),
                     fz);
}

void gen_terrain(i64 seed, Chunk* c)
{
    for (u8 x = 0; x < CHUNK_SIZE; x++)
    {
        for (u8 z = 0; z < CHUNK_SIZE; z++)
        {
            VEC3(float) pos = VEC3_CAST(float, chunk_local_to_world(c->pos, (VEC3(u8)){ x, 0, z }));

            i64 height = noise2d(seed, pos, 16.0f, 40.0f);

            for (u8 y = 0; y < CHUNK_SIZE; y++)
            {
                i64 wy = chunk_local_to_world(c->pos, (VEC3(u8)){ 0, y, 0 }).y;
                BlockType t;
                if (wy > height)
                    t = BLK_AIR;
                else if (wy == height)
                    t = BLK_GRASS;
                else if (wy >= height - 3)
                    t = BLK_DIRT;
                else
                    t = BLK_STONE;
                c->blocks[x + CHUNK_SIZE * (y + CHUNK_SIZE * z)] = (Block){ .type = t };
            }
        }
    }
}
