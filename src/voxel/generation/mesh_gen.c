#include "mesh_gen.h"

#include <string.h>

#include "opengl/tracy.h"
#include "utils/container.h"
#include "voxel/block.h"
#include "voxel/chunk.h"

typedef u32 FaceSlices[BLOCK_COUNT][6][CHUNK_SIZE][CHUNK_SIZE];

static void build_face_slices(FaceSlices slices, bool type_present[BLOCK_COUNT], Chunk* c,
                              Chunk* nbr[6])
{
    for (int z = 0; z < CHUNK_SIZE; z++)
        for (int y = 0; y < CHUNK_SIZE; y++)
            for (int x = 0; x < CHUNK_SIZE; x++)
            {
                BlockType b = c->blocks[CHUNK_IDX(x, y, z)].type;
                if (b == BLK_AIR)
                    continue;
                type_present[b] = true;

                if ((x + 1 < CHUNK_SIZE)
                        ? c->blocks[CHUNK_IDX(x + 1, y, z)].type == BLK_AIR
                        : (!nbr[1] || nbr[1]->blocks[CHUNK_IDX(0, y, z)].type == BLK_AIR))
                    slices[b][0][x][z] |= 1u << y;
                if ((x - 1 >= 0)
                        ? c->blocks[CHUNK_IDX(x - 1, y, z)].type == BLK_AIR
                        : (!nbr[0]
                           || nbr[0]->blocks[CHUNK_IDX(CHUNK_SIZE - 1, y, z)].type == BLK_AIR))
                    slices[b][1][x][z] |= 1u << y;
                if ((y + 1 < CHUNK_SIZE)
                        ? c->blocks[CHUNK_IDX(x, y + 1, z)].type == BLK_AIR
                        : (!nbr[3] || nbr[3]->blocks[CHUNK_IDX(x, 0, z)].type == BLK_AIR))
                    slices[b][2][y][z] |= 1u << x;
                if ((y - 1 >= 0)
                        ? c->blocks[CHUNK_IDX(x, y - 1, z)].type == BLK_AIR
                        : (!nbr[2]
                           || nbr[2]->blocks[CHUNK_IDX(x, CHUNK_SIZE - 1, z)].type == BLK_AIR))
                    slices[b][3][y][z] |= 1u << x;
                if ((z + 1 < CHUNK_SIZE)
                        ? c->blocks[CHUNK_IDX(x, y, z + 1)].type == BLK_AIR
                        : (!nbr[5] || nbr[5]->blocks[CHUNK_IDX(x, y, 0)].type == BLK_AIR))
                    slices[b][4][z][y] |= 1u << x;
                if ((z - 1 >= 0)
                        ? c->blocks[CHUNK_IDX(x, y, z - 1)].type == BLK_AIR
                        : (!nbr[4]
                           || nbr[4]->blocks[CHUNK_IDX(x, y, CHUNK_SIZE - 1)].type == BLK_AIR))
                    slices[b][5][z][y] |= 1u << x;
            }
}

static inline VEC3(i32) get_dir_vector(int x, int y, int dir, int val, int offset)
{
    if ((dir % 2 == 0 && val + offset >= CHUNK_SIZE) || (dir % 2 == 1 && val - offset < 0))
        return (VEC3(i32)){ -1, -1, -1 };

    offset = -offset;

    switch (dir)
    {
    case 0:
        return (VEC3(i32)){ val - offset, x, y };
    case 1:
        return (VEC3(i32)){ val + offset, x, y };
    case 2:
        return (VEC3(i32)){ x, val - offset, y };
    case 3:
        return (VEC3(i32)){ x, val + offset, y };
    case 4:
        return (VEC3(i32)){ x, y, val - offset };
    default:
        return (VEC3(i32)){ x, y, val + offset };
    }
}

static void build_faces(FaceSlices slices, bool present[BLOCK_COUNT], Chunk* c, VECTOR(Face) * buf)
{
    VEC3(i32) base = VEC3_CAST(i32, VEC3_MUL(c->pos, CHUNK_SIZE));

    for (BlockType b = 0; b < BLOCK_COUNT; b++)
    {
        if (b == BLK_AIR || !present[b])
            continue;

        for (int dir = 0; dir < 6; dir++)
            for (int depth = 0; depth < CHUNK_SIZE; depth++)
            {
                // copy the bitset as we ll change it
                u32 row[CHUNK_SIZE];
                memcpy(row, slices[b][dir][depth], sizeof(row));

                for (int v = 0; v < CHUNK_SIZE; v++)
                {
                    while (row[v])
                    {
                        // first bit
                        int u = __builtin_ctz(row[v]);

                        // nb of consecutive 1s at u
                        u32 shifted = row[v] >> u;
                        int w = (shifted == ~0u) ? 32 : __builtin_ctz(~shifted);

                        // expand sideways
                        u32 run = (w == 32) ? ~0u : (((1u << w) - 1u) << u);

                        // expand downward
                        int h = 1;
                        while (v + h < CHUNK_SIZE && (row[v + h] & run) == run)
                            h++;

                        // Clear consumed bits
                        for (int dv = 0; dv < h; dv++)
                            row[v + dv] &= ~run;

                        // Emit face
                        VEC3(i32) pos = get_dir_vector(u, v, dir, depth, 0);
                        VEC3(i32) scale = get_dir_vector(w, h, dir, 1, 0);
                        VECTOR_PUSH_BACK(
                            *buf,
                            ((Face){
                                .face_id = dir,
                                .pos = VEC3_ADD(pos, base),
                                .texture_id = face_texture_resolve(&BlockFaces[b][dir], 0, 0, 0),
                                .scale = scale,
                            }));
                    }
                }
            }
    }
}

static void cache_neighbors(Chunk* c, MAP(ChunkPos, ChunkPtr) * w, Chunk* nbr[6])
{
    static const VEC3(i32) axis[3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };
    for (int a = 0; a < 3; a++)
    {
        ChunkPtr* p;
        p = MAP_GET_T(ChunkPos, ChunkPtr, *w, VEC3_SUB(c->pos, axis[a]));
        nbr[a * 2] = p ? *p : NULL;
        p = MAP_GET_T(ChunkPos, ChunkPtr, *w, VEC3_ADD(c->pos, axis[a]));
        nbr[a * 2 + 1] = p ? *p : NULL;
    }
}

void chunk_to_faces(Chunk* c, MAP(ChunkPos, ChunkPtr) * w, VECTOR(Face) * buf)
{
    TracyZone(ctx, "chunk_to_face");

    Chunk* nbr[6];
    cache_neighbors(c, w, nbr);

    FaceSlices slices = { 0 };
    bool block_type_present[BLOCK_COUNT] = { 0 };

    build_face_slices(slices, block_type_present, c, nbr);
    build_faces(slices, block_type_present, c, buf);

    TracyZoneEnd(ctx);
}
