#include "world.h"

#include <stdio.h>
#include <stdlib.h>

#include "utils/container.h"
#include "utils/vec3.h"
#include "voxel/chunk.h"

World init_world(void)
{
    World w = {
        MAP_INIT(),
        .player = calloc(1, sizeof(Player)),
    };

    VECTOR_INIT(w.chunks);

    return w;
}

static void add_missing_chunk(World* w, VEC3(i64) pos)
{
    ChunkPtr* existing = MAP_GET_T(ChunkPos, ChunkPtr, w->chunks, pos);
    if (existing)
        return;

    Chunk* c = create_random_chunk(pos.x, pos.y, pos.z);
    MAP_INSERT_T(ChunkPos, ChunkPtr, w->chunks, pos, c);
    printf("Adding %zi %zi %zi\n", pos.x, pos.y, pos.z);
}

static void regenerate_faces_buffer(World *w, GLuint vao, size_t* face_count)
{
    glBindVertexArray(vao);

    VECTOR(Face) face_instances;
    VECTOR_INIT(face_instances);

    MAP_FOR_EACH(w->chunks, c)
    {
        face_instances = chunk_to_faces(c->value, face_instances);
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(Face) * VECTOR_SIZE(face_instances), face_instances.data, GL_STATIC_DRAW);

    *face_count = face_instances.size;
}

void generate_new_chunks(World* w, GLuint vao, size_t* face_count)
{
    VEC3(i64) player_chunk_pos = (VEC3(i64)){   ((i64)w->player->pos.x / CHUNK_SIZE),
                                                ((i64)w->player->pos.y / CHUNK_SIZE),
                                                ((i64)w->player->pos.z / CHUNK_SIZE) };

    Chunk** c = MAP_GET_T(ChunkPos, ChunkPtr, w->chunks, player_chunk_pos);

    if (c != NULL && (*c)->visited)
        return;

    for (i64 z = -3; z <= 3; z++)
    for (i64 y = -3; y <= 3; y++)
    for (i64 x = -3; x <= 3; x++)
    {
        VEC3(i64) pos = { x, y, z };
        VEC3_ADD_INPLACE(&pos, player_chunk_pos);

        add_missing_chunk(w, pos);
    }

    if (!c)
        c = MAP_GET_T(ChunkPos, ChunkPtr, w->chunks, player_chunk_pos);

    (*c)->visited = true;

    regenerate_faces_buffer(w, vao, face_count);
}
