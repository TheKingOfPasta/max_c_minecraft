#include "chunk.h"

#include <stdlib.h>

#include "block.h"

Chunk create_random_chunk(i64 x, i64 y, i64 z)
{
    Chunk c = { .pos = (VEC3(i64)){ .x = x, .y = y, .z = z } };

    for (uint8_t i = 0; i < CHUNK_SIZE; i++)
        for (uint8_t j = 0; j < CHUNK_SIZE; j++)
            for (uint8_t k = 0; k < CHUNK_SIZE; k++)
            {
                c.blocks[i * CHUNK_SIZE * CHUNK_SIZE + j * CHUNK_SIZE + k] =
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

VECTOR(Face) chunk_to_faces(const Chunk* chunk)
{
    VECTOR(Face) face_instances;
    VECTOR_INIT(face_instances);

    static const VEC3(i8) dirs[6] = {
        { 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 },
    };

    for (u8 x = 0; x < CHUNK_SIZE; x++)
        for (u8 y = 0; y < CHUNK_SIZE; y++)
            for (u8 z = 0; z < CHUNK_SIZE; z++)
            {
                const Block* b = chunk_get(chunk, (VEC3(u8)){ x, y, z });
                if (b->type == BLK_AIR)
                    continue;

                for (int f = 0; f < 6; f++)
                {
                    VEC3(u8) npos = { x + dirs[f].x, y + dirs[f].y, z + dirs[f].z };

                    if (chunk_is_in_bound(npos) && chunk_get(chunk, npos)->type != BLK_AIR)
                        continue;

                    Face face = (Face){
				.face_id = f,
				.texture_id = face_texture_resolve(
					&BlockFaces[b->type][f], 0, 0, 0),
				.pos =
				{
					.x = (i32)(chunk->pos.x * CHUNK_SIZE + x),
					.y = (i32)(chunk->pos.y * CHUNK_SIZE + y),
					.z = (i32)(chunk->pos.z * CHUNK_SIZE + z),
				},
			};

                    VECTOR_PUSH_BACK(face_instances, face);
                }
            }

    return face_instances;
}
