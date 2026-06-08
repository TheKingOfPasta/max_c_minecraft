#include "gen.h"

#include <stdio.h>

#include "noise.h"
#include "opengl/tracy.h"
#include "utils/vec3.h"
#include "voxel/block.h"
#include "voxel/chunk.h"

#define NOISE_STEP 16

_Static_assert(CHUNK_SIZE % NOISE_STEP == 0, "NOISE_STEP must divide CHUNK_SIZE");

#define NOISE_GRID (CHUNK_SIZE / NOISE_STEP + 1)

typedef float NoiseGrid[NOISE_GRID][NOISE_GRID][NOISE_GRID];

static inline void compute_sample_grid(i64 seed, VEC3(i64) origin, NoiseGrid grid)
{
    for (int gz = 0; gz < NOISE_GRID; gz++)
        for (int gy = 0; gy < NOISE_GRID; gy++)
            for (int gx = 0; gx < NOISE_GRID; gx++)
            {
                VEC3(i32) g = { gx, gy, gz };
                VEC3(i64) gpos = VEC3_ADD(origin, VEC3_MUL(g, NOISE_STEP));
                grid[gz][gy][gx] =
                    noise3d(seed, VEC3_CAST(float, gpos), (VEC3(float)){ 256.0f, 64.0f, 256.0f });
            }
}

static inline float lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

static inline float trilinear(const NoiseGrid grid, VEC3(i32) gi, VEC3(float) t)
{
    float a = lerp(grid[gi.z][gi.y][gi.x], grid[gi.z][gi.y][gi.x + 1], t.x);
    float b = lerp(grid[gi.z][gi.y + 1][gi.x], grid[gi.z][gi.y + 1][gi.x + 1], t.x);
    float c = lerp(grid[gi.z + 1][gi.y][gi.x], grid[gi.z + 1][gi.y][gi.x + 1], t.x);
    float d = lerp(grid[gi.z + 1][gi.y + 1][gi.x], grid[gi.z + 1][gi.y + 1][gi.x + 1], t.x);
    return lerp(lerp(a, b, t.y), lerp(c, d, t.y), t.z);
}

static inline bool compute_solid(const NoiseGrid grid, VEC3(i32) lpos)
{
    VEC3(i32) gi = VEC3_DIV(lpos, NOISE_STEP);
    VEC3(float) t = VEC3_MUL(VEC3_CAST(float, VEC3_MOD(lpos, NOISE_STEP)), 1.0f / NOISE_STEP);
    return trilinear(grid, gi, t) > 0.8f;
}

static inline BlockType paint_block(i64 seed, VEC3(i64) wpos, int depth)
{
    depth += 2 * noise2d(seed, VEC3_CAST(float, wpos), 1);

    if (depth <= 1)
        return BLK_GRASS;
    if (depth <= 3)
        return BLK_DIRT;
    float var =
        noise3d(seed ^ 0x5A3C1B9FL, VEC3_CAST(float, wpos), (VEC3(float)){ 32.0f, 32.0f, 32.0f });
    if (var > 0.85f)
        return BLK_GRAVEL;
    if (var < 0.15f)
        return BLK_DARKSTONE;
    return BLK_STONE;
}

static void paint_terrain(i64 seed, VEC3(i64) origin, Chunk* c)
{
    for (int x = 0; x < CHUNK_SIZE; x++)
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            bool top_solid = c->blocks[CHUNK_IDX(x, CHUNK_SIZE - 1, z)].type != BLK_AIR;
            int depth = top_solid ? 100 : -1;

            for (int y = CHUNK_SIZE - 1; y >= 0; y--)
            {
                Block* b = &c->blocks[CHUNK_IDX(x, y, z)];
                if (b->type == BLK_AIR)
                    continue;

                if (depth < 0)
                    depth = 0;
                else
                    depth++;

                VEC3(i64) wpos = VEC3_ADD(origin, ((VEC3(i64)){ x, y, z }));
                b->type = paint_block(seed, wpos, depth);
            }
        }
}

void gen_terrain(i64 seed, Chunk* c)
{
    TracyZone(ctx, "gen_terrain");

    VEC3(i64) origin = VEC3_MUL(c->pos, CHUNK_SIZE);

    NoiseGrid grid;
    compute_sample_grid(seed, origin, grid);

    for (int z = 0; z < CHUNK_SIZE; z++)
        for (int y = 0; y < CHUNK_SIZE; y++)
            for (int x = 0; x < CHUNK_SIZE; x++)
                c->blocks[CHUNK_IDX(x, y, z)] = (Block){
                    .type = compute_solid(grid, (VEC3(i32)){ x, y, z }) ? BLK_STONE : BLK_AIR,
                };

    paint_terrain(seed, origin, c);

    TracyZoneEnd(ctx);
}
