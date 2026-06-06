#include "chunk.h"

#include <stdlib.h>

#include "block.h"
#include "opengl/tracy.h"
#include "utils/vec3.h"

#define CHUNK_IDX(x, y, z) ((z) * CHUNK_SIZE * CHUNK_SIZE + (y) * CHUNK_SIZE + (x))

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

static const VEC3(i64) dirs[6] = {
    { 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 },
};

static inline void emit_face(int f, const Block* b, VEC3(i32) wpos, Face* buf, size_t* n)
{
    buf[(*n)++] = (Face){
        .face_id = f,
        .texture_id = face_texture_resolve(&BlockFaces[b->type][f], 0, 0, 0),
        .pos = wpos,
    };
}

static inline void border_block_faces(int x, int y, int z, Chunk* c, Chunk** nb, VEC3(i32) base,
                                      Face* buf, size_t* n)
{
    const Block* b = &c->blocks[CHUNK_IDX(x, y, z)];
    if (b->type == BLK_AIR)
        return;

    VEC3(i32) wpos = VEC3_ADD(base, ((VEC3(i32)){ x, y, z }));

    for (int f = 0; f < 6; f++)
    {
        VEC3(i32) nv = VEC3_ADD(((VEC3(i32)){ x, y, z }), VEC3_CAST(i32, dirs[f]));

        if ((unsigned)nv.x < CHUNK_SIZE && (unsigned)nv.y < CHUNK_SIZE
            && (unsigned)nv.z < CHUNK_SIZE)
        {
            if (c->blocks[CHUNK_IDX(nv.x, nv.y, nv.z)].type != BLK_AIR)
                continue;
        }
        else
        {
            if (!nb[f]
                || nb[f]->blocks[CHUNK_IDX(nv.x & (CHUNK_SIZE - 1), nv.y & (CHUNK_SIZE - 1),
                                           nv.z & (CHUNK_SIZE - 1))]
                        .type
                    != BLK_AIR)
                continue;
        }

        emit_face(f, b, wpos, buf, n);
    }
}

void chunk_to_faces(Chunk* c, MAP(ChunkPos, ChunkPtr) * w, Face* buf, size_t* count)
{
    c->face_start_index = *count;

    Chunk* nb[6];
    for (int f = 0; f < 6; f++)
    {
        ChunkPtr* p = MAP_GET_T(ChunkPos, ChunkPtr, *w, VEC3_ADD(dirs[f], c->pos));
        nb[f] = p ? *p : NULL;
    }

    VEC3(i32) base = VEC3_CAST(i32, VEC3_SCALE(c->pos, CHUNK_SIZE));
    size_t n = *count;

    // inside the chunk
    for (int z = 1; z < CHUNK_SIZE - 1; z++)
        for (int y = 1; y < CHUNK_SIZE - 1; y++)
            for (int x = 1; x < CHUNK_SIZE - 1; x++)
            {
                const Block* b = &c->blocks[CHUNK_IDX(x, y, z)];
                if (b->type == BLK_AIR)
                    continue;

                VEC3(i32) wpos = VEC3_ADD(base, ((VEC3(i32)){ x, y, z }));

                for (int f = 0; f < 6; f++)
                {
                    VEC3(i32) nv = VEC3_ADD(((VEC3(i32)){ x, y, z }), VEC3_CAST(i32, dirs[f]));
                    if (c->blocks[CHUNK_IDX(nv.x, nv.y, nv.z)].type != BLK_AIR)
                        continue;

                    emit_face(f, b, wpos, buf, &n);
                }
            }

    // border
    for (int z = 0; z < CHUNK_SIZE; z++)
        for (int y = 0; y < CHUNK_SIZE; y++)
        {
            border_block_faces(0, y, z, c, nb, base, buf, &n);
            border_block_faces(CHUNK_SIZE - 1, y, z, c, nb, base, buf, &n);
        }
    for (int z = 0; z < CHUNK_SIZE; z++)
        for (int x = 1; x < CHUNK_SIZE - 1; x++)
        {
            border_block_faces(x, 0, z, c, nb, base, buf, &n);
            border_block_faces(x, CHUNK_SIZE - 1, z, c, nb, base, buf, &n);
        }
    for (int y = 1; y < CHUNK_SIZE - 1; y++)
        for (int x = 1; x < CHUNK_SIZE - 1; x++)
        {
            border_block_faces(x, y, 0, c, nb, base, buf, &n);
            border_block_faces(x, y, CHUNK_SIZE - 1, c, nb, base, buf, &n);
        }

    c->face_count = n - *count;
    *count = n;
}
