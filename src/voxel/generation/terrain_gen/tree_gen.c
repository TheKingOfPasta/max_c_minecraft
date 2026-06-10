#include "tree_gen.h"

#include "noise.h"
#include "voxel/block.h"

#define TRUNK_H 20
#define BRANCH_H_LO 13
#define BRANCH_H_HI 18
#define BRANCH_LEN 9
#define TREE_SPACING 20
#define TREE_REACH 24

// large-scale noise controls forest patches vs open areas
#define PATCH_SCALE 80.0f
#define PATCH_THRESH 0.5f

static void put_oak(Chunk* c, VEC3(i32) o, i32 wx, i32 wy, i32 wz)
{
    i32 lx = wx - o.x, ly = wy - o.y, lz = wz - o.z;
    if ((u32)lx >= CHUNK_SIZE || (u32)ly >= CHUNK_SIZE || (u32)lz >= CHUNK_SIZE)
        return;
    c->blocks[CHUNK_IDX(lx, ly, lz)].type = BLK_OAK;
}

static void put_leaf(Chunk* c, VEC3(i32) o, i32 wx, i32 wy, i32 wz)
{
    i32 lx = wx - o.x, ly = wy - o.y, lz = wz - o.z;
    if ((u32)lx >= CHUNK_SIZE || (u32)ly >= CHUNK_SIZE || (u32)lz >= CHUNK_SIZE)
        return;
    Block* b = &c->blocks[CHUNK_IDX(lx, ly, lz)];
    if (b->type == BLK_AIR)
        b->type = BLK_FOLIAGE;
}

static void foliage_blob(Chunk* c, VEC3(i32) o, i32 cx, i32 cy, i32 cz, i32 rx, i32 ry, i32 rz)
{
    float irx = 1.0f / (float)(rx * rx);
    float iry = 1.0f / (float)(ry * ry);
    float irz = 1.0f / (float)(rz * rz);
    for (i32 dy = -ry; dy <= ry; dy++)
        for (i32 dz = -rz; dz <= rz; dz++)
            for (i32 dx = -rx; dx <= rx; dx++)
                if ((float)(dx * dx) * irx + (float)(dy * dy) * iry + (float)(dz * dz) * irz
                    <= 1.0f)
                    put_leaf(c, o, cx + dx, cy + dy, cz + dz);
}

static void branch(Chunk* c, VEC3(i32) o, i32 sx, i32 sy, i32 sz, i32 ddx, i32 ddz, i32 len)
{
    for (i32 i = 0; i < len; i++)
    {
        i32 wx = sx + ddx * i;
        i32 wz = sz + ddz * i;
        i32 wy = sy + i / 2;
        if (ddx != 0)
        {
            put_oak(c, o, wx, wy, sz);
            put_oak(c, o, wx, wy, sz + 1);
        }
        else
        {
            put_oak(c, o, sx, wy, wz);
            put_oak(c, o, sx + 1, wy, wz);
        }
    }
    foliage_blob(c, o, sx + ddx * len, sy + len / 2 + 1, sz + ddz * len, 5, 4, 5);
}

// Horizontal drift of the trunk centre at height dy (two-segment curve).
static i32 tdrift(i32 dy, i32 th, i32 l1, i32 l2)
{
    i32 mid = th >> 1;
    if (mid <= 0)
        return 0;
    if (dy <= mid)
        return l1 * dy / mid;
    return l1 + l2 * (dy - mid) / (th - mid);
}

static void place_tree(Chunk* c, VEC3(i32) o, i32 bx, i32 by, i32 bz, i32 th, u64 h)
{
    // Two-segment lean: each half of the trunk bends independently.
    // ltab: -1 (25%), 0 (50%), +1 (25%)
    static const i32 ltab[4] = { -1, 0, 0, 1 };
    i32 lx1 = ltab[(h >> 48) & 3];
    i32 lz1 = ltab[(h >> 50) & 3];
    i32 lx2 = ltab[(h >> 52) & 3];
    i32 lz2 = ltab[(h >> 54) & 3];

    // Thick 4×4 base (rooted, no drift)
    for (i32 dy = 0; dy < 3; dy++)
        for (i32 dx = -1; dx <= 2; dx++)
            for (i32 dz = -1; dz <= 2; dz++)
                put_oak(c, o, bx + dx, by + dy, bz + dz);

    // 2×2 trunk with lean
    for (i32 dy = 0; dy < th; dy++)
    {
        i32 ox = tdrift(dy, th, lx1, lx2);
        i32 oz = tdrift(dy, th, lz1, lz2);
        put_oak(c, o, bx + ox, by + dy, bz + oz);
        put_oak(c, o, bx + ox + 1, by + dy, bz + oz);
        put_oak(c, o, bx + ox, by + dy, bz + oz + 1);
        put_oak(c, o, bx + ox + 1, by + dy, bz + oz + 1);
    }

    // Trunk drift at each attachment height
    i32 dlo_x = tdrift(BRANCH_H_LO, th, lx1, lx2);
    i32 dlo_z = tdrift(BRANCH_H_LO, th, lz1, lz2);
    i32 dhi_x = tdrift(BRANCH_H_HI, th, lx1, lx2);
    i32 dhi_z = tdrift(BRANCH_H_HI, th, lz1, lz2);
    i32 dtop_x = tdrift(th, th, lx1, lx2);
    i32 dtop_z = tdrift(th, th, lz1, lz2);

    // {dx_off, dz_off, ddx, ddz} per cardinal direction
    static const i32 bd[4][4] = {
        { 2, 0, 1, 0 },
        { -1, 0, -1, 0 },
        { 0, 2, 0, 1 },
        { 0, -1, 0, -1 },
    };

    for (int d = 0; d < 4; d++)
    {
        // Independent per-branch randomness
        u32 bh = (u32)h ^ (u32)(d * 0x9e3779b9u);
        bh ^= bh >> 16;
        bh *= 0x45d9f3bu;
        bh ^= bh >> 16;

        // Lower tier: each direction has 75% chance, random length ±2
        if ((bh & 3) != 0)
        {
            i32 len = BRANCH_LEN + (i32)((bh >> 4) & 3) - 1;
            branch(c, o, bx + dlo_x + bd[d][0], by + BRANCH_H_LO, bz + dlo_z + bd[d][1], bd[d][2],
                   bd[d][3], len);
        }

        // Upper tier: 75% chance, shorter, random length ±1
        if (((bh >> 2) & 3) != 0)
        {
            i32 len = (BRANCH_LEN - 2) + (i32)((bh >> 6) & 3) - 1;
            if (len < 2)
                len = 2;
            branch(c, o, bx + dhi_x + bd[d][0], by + BRANCH_H_HI, bz + dhi_z + bd[d][1], bd[d][2],
                   bd[d][3], len);
        }
    }

    // Crown follows the trunk top
    foliage_blob(c, o, bx + dtop_x, by + th + 3, bz + dtop_z, 10, 7, 10);
}

static i32 find_surface_wy(i32 lx, i32 lz, i32 trunk_cx, i32 trunk_cz, i32 cy_lo, i32 cy_hi,
                           MAP(ChunkPos, ChunkPtr) * world)
{
    bool hit_solid = false;
    for (i32 cy = cy_hi; cy >= cy_lo && !hit_solid; cy--)
    {
        ChunkPtr* cp =
            MAP_GET_T(ChunkPos, ChunkPtr, *world, ((VEC3(i32)){ trunk_cx, cy, trunk_cz }));
        if (!cp)
            continue;
        Chunk* tc = *cp;
        for (i32 ly = CHUNK_SIZE - 1; ly >= 0; ly--)
        {
            BlockType t = tc->blocks[CHUNK_IDX(lx, ly, lz)].type;
            if (t == BLK_AIR || t == BLK_OAK || t == BLK_FOLIAGE)
                continue;
            hit_solid = true;
            if (t == BLK_GRASS)
                return cy * CHUNK_SIZE + ly;
            break;
        }
    }
    return (i32)0x80000000;
}

void gen_trees(i32 seed, Chunk* c, MAP(ChunkPos, ChunkPtr) * world)
{
    VEC3(i32) o = VEC3_MUL(c->pos, CHUNK_SIZE);

    i32 g0x = (o.x - TREE_REACH) / TREE_SPACING - 2;
    i32 g1x = (o.x + CHUNK_SIZE + TREE_REACH) / TREE_SPACING + 2;
    i32 g0z = (o.z - TREE_REACH) / TREE_SPACING - 2;
    i32 g1z = (o.z + CHUNK_SIZE + TREE_REACH) / TREE_SPACING + 2;

    for (i32 gx = g0x; gx <= g1x; gx++)
        for (i32 gz = g0z; gz <= g1z; gz++)
        {
            u64 h = (u64)((i64)gx * 1619 + (i64)gz * 31337 + (i64)seed * 6971);
            h ^= h >> 17;
            h *= 0xbf58476d1ce4e5b9ULL;
            h ^= h >> 31;

            if (h % 2 == 0)
                continue; // 50% of slots empty within patches

            i32 wx = gx * TREE_SPACING + (i32)((h >> 8) % (u64)TREE_SPACING);
            i32 wz = gz * TREE_SPACING + (i32)((h >> 20) % (u64)TREE_SPACING);

            // Forest patch filter: large-scale noise creates dense vs open areas
            float cluster =
                noise2d(seed ^ 0xF00D, (VEC3(float)){ (float)wx, 0.0f, (float)wz }, PATCH_SCALE);
            if (cluster < PATCH_THRESH)
                continue;

            i32 trunk_cx = wx >> CHUNK_SHIFT;
            i32 trunk_cz = wz >> CHUNK_SHIFT;
            i32 lx = wx & (CHUNK_SIZE - 1);
            i32 lz = wz & (CHUNK_SIZE - 1);

            i32 surf_wy =
                find_surface_wy(lx, lz, trunk_cx, trunk_cz, c->pos.y - 1, c->pos.y, world);
            if (surf_wy == (i32)0x80000000)
                continue;

            // Height: TRUNK_H ± 4, never below 20
            i32 th = TRUNK_H + (i32)((h >> 32) % 16u) - 8;
            if (th < 10)
                th = 10;

            place_tree(c, o, wx, surf_wy + 1, wz, th, h);
        }
}
