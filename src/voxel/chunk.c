#include "chunk.h"

#include <stdlib.h>

#include "block.h"
#include "opengl/tracy.h"
#include "utils/vec3.h"

Chunk* create_empty_chunk(VEC3(i64) pos)
{
    Chunk* c = calloc(1, sizeof(Chunk));
    c->pos = pos;

    return c;
}

Chunk* create_random_chunk(i64 x, i64 y, i64 z)
{
    Chunk* c = calloc(1, sizeof(Chunk));
    c->pos = (VEC3(i64)){ .x = x, .y = y, .z = z };

    for (uint8_t i = 0; i < CHUNK_SIZE; i++)
        for (uint8_t j = 0; j < CHUNK_SIZE; j++)
            for (uint8_t k = 0; k < CHUNK_SIZE; k++)
                c->blocks[CHUNK_IDX(k, j, i)] = (Block){ .type = rand() % BLOCK_COUNT };

    return c;
}

const Block* chunk_get(const Chunk* chunk, VEC3(u8) pos)
{
    return chunk->blocks + CHUNK_IDX(pos.x, pos.y, pos.z);
}

static const VEC3(i32) dirs[6] = {
    { 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 },
};

// match shader's lookup
static const int faceVerts[6][4][3] = {
    { { 1, 0, 0 }, { 1, 1, 0 }, { 1, 1, 1 }, { 1, 0, 1 } }, // +X
    { { 0, 0, 1 }, { 0, 1, 1 }, { 0, 1, 0 }, { 0, 0, 0 } }, // -X
    { { 0, 1, 1 }, { 1, 1, 1 }, { 1, 1, 0 }, { 0, 1, 0 } }, // +Y
    { { 0, 0, 0 }, { 1, 0, 0 }, { 1, 0, 1 }, { 0, 0, 1 } }, // -Y
    { { 1, 0, 1 }, { 1, 1, 1 }, { 0, 1, 1 }, { 0, 0, 1 } }, // +Z
    { { 0, 0, 0 }, { 0, 1, 0 }, { 1, 1, 0 }, { 1, 0, 0 } }, // -Z
};

static inline bool is_world_block_solid(VEC3(i32) wpos, MAP(ChunkPos, ChunkPtr) * w)
{
    VEC3(i64) ch_pos = VEC3_CAST(i64, VEC3_RBS(wpos, CHUNK_SHIFT));
    ChunkPtr* p = MAP_GET_T(ChunkPos, ChunkPtr, *w, ch_pos);
    if (!p)
        return false;
    VEC3(i32) bpos = VEC3_AND(wpos, CHUNK_SIZE - 1);
    return (*p)->blocks[CHUNK_IDX(bpos.x, bpos.y, bpos.z)].type != BLK_AIR;
}

static inline int vertex_ao(int f, int v, VEC3(i32) wpos, MAP(ChunkPos, ChunkPtr) * w)
{
    VEC3(i32) n = dirs[f];
    const int* fv = faceVerts[f][v];

    VEC3(i32) t1, t2;
    if (n.x != 0)
    {
        t1 = (VEC3(i32)){ 0, fv[1] * 2 - 1, 0 };
        t2 = (VEC3(i32)){ 0, 0, fv[2] * 2 - 1 };
    }
    else if (n.y != 0)
    {
        t1 = (VEC3(i32)){ fv[0] * 2 - 1, 0, 0 };
        t2 = (VEC3(i32)){ 0, 0, fv[2] * 2 - 1 };
    }
    else
    {
        t1 = (VEC3(i32)){ fv[0] * 2 - 1, 0, 0 };
        t2 = (VEC3(i32)){ 0, fv[1] * 2 - 1, 0 };
    }

    VEC3(i32) base = VEC3_ADD(wpos, n);
    VEC3(i32) side1 = VEC3_ADD(base, t1);
    VEC3(i32) side2 = VEC3_ADD(base, t2);
    VEC3(i32) corner = VEC3_ADD(side1, t2);

    bool s1 = is_world_block_solid(side1, w);
    bool s2 = is_world_block_solid(side2, w);
    bool co = is_world_block_solid(corner, w);

    if (s1 && s2)
        return 0;
    return 3 - s1 - s2 - co;
}

static inline void emit_face(int f, const Block* b, VEC3(i32) wpos, VECTOR(Face) * buf,
                             MAP(ChunkPos, ChunkPtr) * w)
{
    int ao0 = vertex_ao(f, 0, wpos, w);
    int ao1 = vertex_ao(f, 1, wpos, w);
    int ao2 = vertex_ao(f, 2, wpos, w);
    int ao3 = vertex_ao(f, 3, wpos, w);
    int flip = (ao0 + ao2 < ao1 + ao3) ? 1 : 0;
    VECTOR_PUSH_BACK(
        *buf,
        ((Face){
            .face_id = f | (flip << 3) | (ao0 << 4) | (ao1 << 6) | (ao2 << 8) | (ao3 << 10),
            .texture_id = face_texture_resolve(&BlockFaces[b->type][f], 0, 0, 0),
            .pos = wpos,
        }));
}

static inline void border_block_faces(VEC3(i32) lpos, Chunk* c, Chunk** neighbours, VEC3(i32) base,
                                      VECTOR(Face) * buf, MAP(ChunkPos, ChunkPtr) * w)
{
    const Block* b = &c->blocks[CHUNK_IDX(lpos.x, lpos.y, lpos.z)];
    if (b->type == BLK_AIR)
        return;

    VEC3(i32) wpos = VEC3_ADD(base, lpos);

    for (int f = 0; f < 6; f++)
    {
        VEC3(i32) nv = VEC3_ADD(lpos, dirs[f]);
        if ((unsigned)nv.x < CHUNK_SIZE && (unsigned)nv.y < CHUNK_SIZE
            && (unsigned)nv.z < CHUNK_SIZE)
        {
            if (c->blocks[CHUNK_IDX(nv.x, nv.y, nv.z)].type != BLK_AIR)
                continue;
        }
        else
        {
            if (!neighbours[f]
                || neighbours[f]
                        ->blocks[CHUNK_IDX(nv.x & (CHUNK_SIZE - 1), nv.y & (CHUNK_SIZE - 1),
                                           nv.z & (CHUNK_SIZE - 1))]
                        .type
                    != BLK_AIR)
                continue;
        }

        emit_face(f, b, wpos, buf, w);
    }
}

void chunk_to_faces(Chunk* c, MAP(ChunkPos, ChunkPtr) * w, VECTOR(Face) * buf)
{
    TracyZone(ctx, "chunk_to_face");

    Chunk* nb[6];
    for (int f = 0; f < 6; f++)
    {
        ChunkPtr* p = MAP_GET_T(ChunkPos, ChunkPtr, *w, VEC3_ADD(c->pos, VEC3_CAST(i64, dirs[f])));
        nb[f] = p ? *p : NULL;
    }

    VEC3(i32) base = VEC3_CAST(i32, VEC3_MUL(c->pos, CHUNK_SIZE));

    for (int z = 1; z < CHUNK_SIZE - 1; z++)
        for (int y = 1; y < CHUNK_SIZE - 1; y++)
            for (int x = 1; x < CHUNK_SIZE - 1; x++)
            {
                const Block* b = &c->blocks[CHUNK_IDX(x, y, z)];
                if (b->type == BLK_AIR)
                    continue;

                VEC3(i32) lpos = { x, y, z };
                VEC3(i32) wpos = VEC3_ADD(base, lpos);
                for (int f = 0; f < 6; f++)
                {
                    VEC3(i32) nv = VEC3_ADD(lpos, dirs[f]);
                    if (c->blocks[CHUNK_IDX(nv.x, nv.y, nv.z)].type != BLK_AIR)
                        continue;
                    emit_face(f, b, wpos, buf, w);
                }
            }

    for (int z = 0; z < CHUNK_SIZE; z++)
        for (int y = 0; y < CHUNK_SIZE; y++)
        {
            border_block_faces((VEC3(i32)){ 0, y, z }, c, nb, base, buf, w);
            border_block_faces((VEC3(i32)){ CHUNK_SIZE - 1, y, z }, c, nb, base, buf, w);
        }
    for (int z = 0; z < CHUNK_SIZE; z++)
        for (int x = 1; x < CHUNK_SIZE - 1; x++)
        {
            border_block_faces((VEC3(i32)){ x, 0, z }, c, nb, base, buf, w);
            border_block_faces((VEC3(i32)){ x, CHUNK_SIZE - 1, z }, c, nb, base, buf, w);
        }
    for (int y = 1; y < CHUNK_SIZE - 1; y++)
        for (int x = 1; x < CHUNK_SIZE - 1; x++)
        {
            border_block_faces((VEC3(i32)){ x, y, 0 }, c, nb, base, buf, w);
            border_block_faces((VEC3(i32)){ x, y, CHUNK_SIZE - 1 }, c, nb, base, buf, w);
        }

    TracyZoneEnd(ctx);
}
