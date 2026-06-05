#include "world.h"

#include <stdio.h>
#include <stdlib.h>

#include "utils/container.h"
#include "utils/vec3.h"
#include "voxel/chunk.h"
#include "voxel/terrain_gen/gen.h"

VECTOR(Face) face_instances;

World init_world(void)
{
    World w = {
        MAP_INIT(),
        .player = calloc(1, sizeof(Player)),
    };

    VECTOR_INIT(w.chunks);
    VECTOR_INIT(face_instances);
    VECTOR_INIT(w.drawn_chunks);

    return w;
}

static void add_missing_chunk(World* w, VEC3(i64) pos)
{
    ChunkPtr* existing = MAP_GET_T(ChunkPos, ChunkPtr, w->chunks, pos);
    if (existing)
        return;

    Chunk* c = create_empty_chunk(pos);

    gen_terrain(42, c);

    MAP_INSERT_T(ChunkPos, ChunkPtr, w->chunks, pos, c);
}

static void regenerate_faces_buffer([[maybe_unused]]World* w, GLuint vao, size_t* face_count)
{
    glBindVertexArray(vao);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Face) * VECTOR_SIZE(face_instances), face_instances.data,
                 GL_DYNAMIC_DRAW);

    *face_count = face_instances.size;
}

static void generate_new_chunks_border(World* w, VEC3(i64) diff)
{
    if (diff.x > 0)
    {
        for (i64 z = -LOADED_CHUNK_DISTANCE; z <= LOADED_CHUNK_DISTANCE; z++)
        for (i64 y = -LOADED_CHUNK_DISTANCE; y <= LOADED_CHUNK_DISTANCE; y++)
        for (i64 x = LOADED_CHUNK_DISTANCE + 1; x <= LOADED_CHUNK_DISTANCE + diff.x; x++)
        {
            VEC3(i64) pos = { x, y, z };
            VEC3_ADD_INPLACE(&pos, w->old_chunk_pos);

            add_missing_chunk(w, pos);
        }
        for (i64 z = -LOADED_CHUNK_DISTANCE; z <= LOADED_CHUNK_DISTANCE; z++)
        for (i64 y = -LOADED_CHUNK_DISTANCE; y <= LOADED_CHUNK_DISTANCE; y++)
        for (i64 x = LOADED_CHUNK_DISTANCE + 1; x <= LOADED_CHUNK_DISTANCE + diff.x; x++)
        {
            VEC3(i64) pos = { x, y, z };
            VEC3_ADD_INPLACE(&pos, w->old_chunk_pos);

            face_instances = chunk_to_faces(*MAP_GET_T(ChunkPos, ChunkPtr, w->chunks, pos), &w->chunks, face_instances);
        }
    }
    else if (diff.x < 0)
    {
        for (i64 z = -LOADED_CHUNK_DISTANCE; z <= LOADED_CHUNK_DISTANCE; z++)
        for (i64 y = -LOADED_CHUNK_DISTANCE; y <= LOADED_CHUNK_DISTANCE; y++)
        for (i64 x = -LOADED_CHUNK_DISTANCE + diff.x; x <= -LOADED_CHUNK_DISTANCE - 1; x++)
        {
            VEC3(i64) pos = { x, y, z };
            VEC3_ADD_INPLACE(&pos, w->old_chunk_pos);

            add_missing_chunk(w, pos);
        }
        for (i64 z = -LOADED_CHUNK_DISTANCE; z <= LOADED_CHUNK_DISTANCE; z++)
        for (i64 y = -LOADED_CHUNK_DISTANCE; y <= LOADED_CHUNK_DISTANCE; y++)
        for (i64 x = -LOADED_CHUNK_DISTANCE + diff.x; x <= -LOADED_CHUNK_DISTANCE - 1; x++)
        {
            VEC3(i64) pos = { x, y, z };
            VEC3_ADD_INPLACE(&pos, w->old_chunk_pos);

            face_instances = chunk_to_faces(*MAP_GET_T(ChunkPos, ChunkPtr, w->chunks, pos), &w->chunks, face_instances);
        }
    }

    if (diff.y > 0)
    {
        for (i64 z = -LOADED_CHUNK_DISTANCE; z <= LOADED_CHUNK_DISTANCE; z++)
        for (i64 y = LOADED_CHUNK_DISTANCE + 1; y <= LOADED_CHUNK_DISTANCE + diff.y; y++)
        for (i64 x = -LOADED_CHUNK_DISTANCE; x <= LOADED_CHUNK_DISTANCE; x++)
        {
            VEC3(i64) pos = { x, y, z };
            VEC3_ADD_INPLACE(&pos, w->old_chunk_pos);

            add_missing_chunk(w, pos);
        }
        for (i64 z = -LOADED_CHUNK_DISTANCE; z <= LOADED_CHUNK_DISTANCE; z++)
        for (i64 y = LOADED_CHUNK_DISTANCE + 1; y <= LOADED_CHUNK_DISTANCE + diff.y; y++)
        for (i64 x = -LOADED_CHUNK_DISTANCE; x <= LOADED_CHUNK_DISTANCE; x++)
        {
            VEC3(i64) pos = { x, y, z };
            VEC3_ADD_INPLACE(&pos, w->old_chunk_pos);

            face_instances = chunk_to_faces(*MAP_GET_T(ChunkPos, ChunkPtr, w->chunks, pos), &w->chunks, face_instances);
        }
    }
    else if (diff.y < 0)
    {
        for (i64 z = -LOADED_CHUNK_DISTANCE; z <= LOADED_CHUNK_DISTANCE; z++)
        for (i64 y = -LOADED_CHUNK_DISTANCE + diff.y; y <= -LOADED_CHUNK_DISTANCE - 1; y++)
        for (i64 x = -LOADED_CHUNK_DISTANCE; x <= LOADED_CHUNK_DISTANCE; x++)
        {
            VEC3(i64) pos = { x, y, z };
            VEC3_ADD_INPLACE(&pos, w->old_chunk_pos);

            add_missing_chunk(w, pos);
        }
        for (i64 z = -LOADED_CHUNK_DISTANCE; z <= LOADED_CHUNK_DISTANCE; z++)
        for (i64 y = -LOADED_CHUNK_DISTANCE + diff.y; y <= -LOADED_CHUNK_DISTANCE - 1; y++)
        for (i64 x = -LOADED_CHUNK_DISTANCE; x <= LOADED_CHUNK_DISTANCE; x++)
        {
            VEC3(i64) pos = { x, y, z };
            VEC3_ADD_INPLACE(&pos, w->old_chunk_pos);

            face_instances = chunk_to_faces(*MAP_GET_T(ChunkPos, ChunkPtr, w->chunks, pos), &w->chunks, face_instances);
        }
    }

    if (diff.z > 0)
    {
        for (i64 z = LOADED_CHUNK_DISTANCE + 1; z <= LOADED_CHUNK_DISTANCE + diff.z; z++)
        for (i64 y = -LOADED_CHUNK_DISTANCE; y <= LOADED_CHUNK_DISTANCE; y++)
        for (i64 x = -LOADED_CHUNK_DISTANCE; x <= LOADED_CHUNK_DISTANCE; x++)
        {
            VEC3(i64) pos = { x, y, z };
            VEC3_ADD_INPLACE(&pos, w->old_chunk_pos);

            add_missing_chunk(w, pos);
        }
        for (i64 z = LOADED_CHUNK_DISTANCE + 1; z <= LOADED_CHUNK_DISTANCE + diff.z; z++)
        for (i64 y = -LOADED_CHUNK_DISTANCE; y <= LOADED_CHUNK_DISTANCE; y++)
        for (i64 x = -LOADED_CHUNK_DISTANCE; x <= LOADED_CHUNK_DISTANCE; x++)
        {
            VEC3(i64) pos = { x, y, z };
            VEC3_ADD_INPLACE(&pos, w->old_chunk_pos);

            face_instances = chunk_to_faces(*MAP_GET_T(ChunkPos, ChunkPtr, w->chunks, pos), &w->chunks, face_instances);
        }
    }
    else if (diff.z < 0)
    {
        for (i64 z = -LOADED_CHUNK_DISTANCE + diff.z; z <= -LOADED_CHUNK_DISTANCE - 1; z++)
        for (i64 y = -LOADED_CHUNK_DISTANCE; y <= LOADED_CHUNK_DISTANCE; y++)
        for (i64 x = -LOADED_CHUNK_DISTANCE; x <= LOADED_CHUNK_DISTANCE; x++)
        {
            VEC3(i64) pos = { x, y, z };
            VEC3_ADD_INPLACE(&pos, w->old_chunk_pos);

            add_missing_chunk(w, pos);
        }
        for (i64 z = -LOADED_CHUNK_DISTANCE + diff.z; z <= -LOADED_CHUNK_DISTANCE - 1; z++)
        for (i64 y = -LOADED_CHUNK_DISTANCE; y <= LOADED_CHUNK_DISTANCE; y++)
        for (i64 x = -LOADED_CHUNK_DISTANCE; x <= LOADED_CHUNK_DISTANCE; x++)
        {
            VEC3(i64) pos = { x, y, z };
            VEC3_ADD_INPLACE(&pos, w->old_chunk_pos);

            face_instances = chunk_to_faces(*MAP_GET_T(ChunkPos, ChunkPtr, w->chunks, pos), &w->chunks, face_instances);
        }
    }
}

void generate_new_chunks(World* w, GLuint vao, size_t* face_count)
{
    VEC3(i64)
    player_chunk_pos =
        (VEC3(i64)){ ((i64)w->player->pos.x / CHUNK_SIZE), ((i64)w->player->pos.y / CHUNK_SIZE),
                     ((i64)w->player->pos.z / CHUNK_SIZE) };

    VEC3(i64) diff = VEC3_SUB(player_chunk_pos, w->old_chunk_pos);

    Chunk** c = MAP_GET_T(ChunkPos, ChunkPtr, w->chunks, player_chunk_pos);

    if (c != NULL && (*c)->visited)
        return;

    if (!c)
    {
        printf("Generating all surrounding chunks\n");
        for (i64 z = -LOADED_CHUNK_DISTANCE; z <= LOADED_CHUNK_DISTANCE; z++)
        for (i64 y = -LOADED_CHUNK_DISTANCE; y <= LOADED_CHUNK_DISTANCE; y++)
        for (i64 x = -LOADED_CHUNK_DISTANCE; x <= LOADED_CHUNK_DISTANCE; x++)
        {
            VEC3(i64) pos = { x, y, z };
            VEC3_ADD_INPLACE(&pos, player_chunk_pos);

            add_missing_chunk(w, pos);
        }
        for (i64 z = -LOADED_CHUNK_DISTANCE; z <= LOADED_CHUNK_DISTANCE; z++)
        for (i64 y = -LOADED_CHUNK_DISTANCE; y <= LOADED_CHUNK_DISTANCE; y++)
        for (i64 x = -LOADED_CHUNK_DISTANCE; x <= LOADED_CHUNK_DISTANCE; x++)
        {
            VEC3(i64) pos = { x, y, z };
            VEC3_ADD_INPLACE(&pos, player_chunk_pos);

            face_instances = chunk_to_faces(*MAP_GET_T(ChunkPos, ChunkPtr, w->chunks, pos), &w->chunks, face_instances);
        }

        c = MAP_GET_T(ChunkPos, ChunkPtr, w->chunks, player_chunk_pos);
    }
    else
    {
        generate_new_chunks_border(w, diff);
    }

    w->old_chunk_pos = player_chunk_pos;

    (*c)->visited = true;

    regenerate_faces_buffer(w, vao, face_count);
}
