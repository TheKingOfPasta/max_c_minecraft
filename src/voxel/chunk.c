#include "chunk.h"

#include <stdlib.h>

#include "block.h"
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
            {
                c->blocks[i * CHUNK_SIZE * CHUNK_SIZE + j * CHUNK_SIZE + k] =
                    (Block){ .type = rand() % BLOCK_COUNT };
            }

    return c;
}

const Block* chunk_get(const Chunk* chunk, VEC3(u8) pos)
{
    const Block* b = chunk->blocks + pos.z * CHUNK_SIZE * CHUNK_SIZE + pos.y * CHUNK_SIZE + pos.x;

    return b;
}

static bool chunk_is_in_bound(VEC3(u8) pos)
{
    return pos.x < CHUNK_SIZE && pos.y < CHUNK_SIZE && pos.z < CHUNK_SIZE;
}

VECTOR(Face)
chunk_to_faces(const Chunk* c, MAP(ChunkPos, ChunkPtr) * w, VECTOR(Face) face_instances)
{
    static const VEC3(i64) dirs[6] = {
        { 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 },
    };

    for (u8 x = 0; x < CHUNK_SIZE; x++)
        for (u8 y = 0; y < CHUNK_SIZE; y++)
            for (u8 z = 0; z < CHUNK_SIZE; z++)
            {
                const Block* b = chunk_get(c, (VEC3(u8)){ x, y, z });
                if (b->type == BLK_AIR)
                    continue;

                for (int f = 0; f < 6; f++)
                {
                    VEC3(u8) npos = { x + dirs[f].x, y + dirs[f].y, z + dirs[f].z };

                    if (chunk_is_in_bound(npos))
                    {
                        if (chunk_get(c, npos)->type != BLK_AIR)
                            continue;
                    }
                    else
                    {
                        VEC3(i64) neighbor_chunk_pos = VEC3_ADD(dirs[f], c->pos);

                        ChunkPtr* neighbor = MAP_GET_T(ChunkPos, ChunkPtr, *w, neighbor_chunk_pos);
                        if (neighbor)
                        {
                            VEC3(u8)
                            local = {
                                dirs[f].x == 1 ? 0 : (dirs[f].x == -1 ? CHUNK_SIZE - 1 : x),
                                dirs[f].y == 1 ? 0 : (dirs[f].y == -1 ? CHUNK_SIZE - 1 : y),
                                dirs[f].z == 1 ? 0 : (dirs[f].z == -1 ? CHUNK_SIZE - 1 : z),
                            };
                            if (chunk_get(*neighbor, local)->type != BLK_AIR)
                                continue;
                        }
                    }

                    Face face = (Face){
                        .face_id = f,
                        .texture_id = face_texture_resolve(&BlockFaces[b->type][f], 0, 0, 0),
                        .pos = VEC3_CAST(i32, chunk_local_to_world(c->pos, (VEC3(u8)){ x, y, z })),
                    };

                    VECTOR_PUSH_BACK(face_instances, face);
                }
            }

    return face_instances;
}
