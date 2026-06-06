#include "world.h"

#include <glad/glad.h>
// glad before
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>

#include "opengl/tracy.h"
#include "utils/container.h"
#include "utils/vec3.h"
#include "voxel/chunk.h"
#include "voxel/terrain_gen/gen.h"

World init_world(void)
{
    World w = {
        MAP_INIT(),
        .player = calloc(1, sizeof(Player)),
    };

    VECTOR_INIT(w.chunks);
    VECTOR_INIT(w.drawn_chunks);

    return w;
}

void world_init_gl(World* w, GLuint face_vbo)
{
    w->face_vbo = face_vbo;
    w->mapped_faces = malloc(MAX_FACE_COUNT * sizeof(Face));
    glBindBuffer(GL_ARRAY_BUFFER, face_vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_FACE_COUNT * sizeof(Face), NULL, GL_DYNAMIC_DRAW);
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

static void generate_new_chunks_border(World* w, VEC3(i64) diff)
{
    if (diff.x > 0)
    {
        for (i64 z = -LOADED_CHUNK_DISTANCE; z <= LOADED_CHUNK_DISTANCE; z++)
            for (i64 y = -LOADED_CHUNK_DISTANCE; y <= LOADED_CHUNK_DISTANCE; y++)
                for (i64 x = LOADED_CHUNK_DISTANCE + 1; x <= LOADED_CHUNK_DISTANCE + diff.x; x++)
                    add_missing_chunk(w, VEC3_ADD(((VEC3(i64)){ x, y, z }), w->old_chunk_pos));
        for (i64 z = -LOADED_CHUNK_DISTANCE; z <= LOADED_CHUNK_DISTANCE; z++)
            for (i64 y = -LOADED_CHUNK_DISTANCE; y <= LOADED_CHUNK_DISTANCE; y++)
                for (i64 x = LOADED_CHUNK_DISTANCE + 1; x <= LOADED_CHUNK_DISTANCE + diff.x; x++)
                    chunk_to_faces(*MAP_GET_T(ChunkPos, ChunkPtr, w->chunks,
                                              VEC3_ADD(((VEC3(i64)){ x, y, z }), w->old_chunk_pos)),
                                   &w->chunks, w->mapped_faces, &w->gpu_face_count);
    }
    else if (diff.x < 0)
    {
        for (i64 z = -LOADED_CHUNK_DISTANCE; z <= LOADED_CHUNK_DISTANCE; z++)
            for (i64 y = -LOADED_CHUNK_DISTANCE; y <= LOADED_CHUNK_DISTANCE; y++)
                for (i64 x = -LOADED_CHUNK_DISTANCE + diff.x; x <= -LOADED_CHUNK_DISTANCE - 1; x++)
                    add_missing_chunk(w, VEC3_ADD(((VEC3(i64)){ x, y, z }), w->old_chunk_pos));
        for (i64 z = -LOADED_CHUNK_DISTANCE; z <= LOADED_CHUNK_DISTANCE; z++)
            for (i64 y = -LOADED_CHUNK_DISTANCE; y <= LOADED_CHUNK_DISTANCE; y++)
                for (i64 x = -LOADED_CHUNK_DISTANCE + diff.x; x <= -LOADED_CHUNK_DISTANCE - 1; x++)
                    chunk_to_faces(*MAP_GET_T(ChunkPos, ChunkPtr, w->chunks,
                                              VEC3_ADD(((VEC3(i64)){ x, y, z }), w->old_chunk_pos)),
                                   &w->chunks, w->mapped_faces, &w->gpu_face_count);
    }

    if (diff.y > 0)
    {
        for (i64 z = -LOADED_CHUNK_DISTANCE; z <= LOADED_CHUNK_DISTANCE; z++)
            for (i64 y = LOADED_CHUNK_DISTANCE + 1; y <= LOADED_CHUNK_DISTANCE + diff.y; y++)
                for (i64 x = -LOADED_CHUNK_DISTANCE; x <= LOADED_CHUNK_DISTANCE; x++)
                    add_missing_chunk(w, VEC3_ADD(((VEC3(i64)){ x, y, z }), w->old_chunk_pos));
        for (i64 z = -LOADED_CHUNK_DISTANCE; z <= LOADED_CHUNK_DISTANCE; z++)
            for (i64 y = LOADED_CHUNK_DISTANCE + 1; y <= LOADED_CHUNK_DISTANCE + diff.y; y++)
                for (i64 x = -LOADED_CHUNK_DISTANCE; x <= LOADED_CHUNK_DISTANCE; x++)
                    chunk_to_faces(*MAP_GET_T(ChunkPos, ChunkPtr, w->chunks,
                                              VEC3_ADD(((VEC3(i64)){ x, y, z }), w->old_chunk_pos)),
                                   &w->chunks, w->mapped_faces, &w->gpu_face_count);
    }
    else if (diff.y < 0)
    {
        for (i64 z = -LOADED_CHUNK_DISTANCE; z <= LOADED_CHUNK_DISTANCE; z++)
            for (i64 y = -LOADED_CHUNK_DISTANCE + diff.y; y <= -LOADED_CHUNK_DISTANCE - 1; y++)
                for (i64 x = -LOADED_CHUNK_DISTANCE; x <= LOADED_CHUNK_DISTANCE; x++)
                    add_missing_chunk(w, VEC3_ADD(((VEC3(i64)){ x, y, z }), w->old_chunk_pos));
        for (i64 z = -LOADED_CHUNK_DISTANCE; z <= LOADED_CHUNK_DISTANCE; z++)
            for (i64 y = -LOADED_CHUNK_DISTANCE + diff.y; y <= -LOADED_CHUNK_DISTANCE - 1; y++)
                for (i64 x = -LOADED_CHUNK_DISTANCE; x <= LOADED_CHUNK_DISTANCE; x++)
                    chunk_to_faces(*MAP_GET_T(ChunkPos, ChunkPtr, w->chunks,
                                              VEC3_ADD(((VEC3(i64)){ x, y, z }), w->old_chunk_pos)),
                                   &w->chunks, w->mapped_faces, &w->gpu_face_count);
    }

    if (diff.z > 0)
    {
        for (i64 z = LOADED_CHUNK_DISTANCE + 1; z <= LOADED_CHUNK_DISTANCE + diff.z; z++)
            for (i64 y = -LOADED_CHUNK_DISTANCE; y <= LOADED_CHUNK_DISTANCE; y++)
                for (i64 x = -LOADED_CHUNK_DISTANCE; x <= LOADED_CHUNK_DISTANCE; x++)
                    add_missing_chunk(w, VEC3_ADD(((VEC3(i64)){ x, y, z }), w->old_chunk_pos));
        for (i64 z = LOADED_CHUNK_DISTANCE + 1; z <= LOADED_CHUNK_DISTANCE + diff.z; z++)
            for (i64 y = -LOADED_CHUNK_DISTANCE; y <= LOADED_CHUNK_DISTANCE; y++)
                for (i64 x = -LOADED_CHUNK_DISTANCE; x <= LOADED_CHUNK_DISTANCE; x++)
                    chunk_to_faces(*MAP_GET_T(ChunkPos, ChunkPtr, w->chunks,
                                              VEC3_ADD(((VEC3(i64)){ x, y, z }), w->old_chunk_pos)),
                                   &w->chunks, w->mapped_faces, &w->gpu_face_count);
    }
    else if (diff.z < 0)
    {
        for (i64 z = -LOADED_CHUNK_DISTANCE + diff.z; z <= -LOADED_CHUNK_DISTANCE - 1; z++)
            for (i64 y = -LOADED_CHUNK_DISTANCE; y <= LOADED_CHUNK_DISTANCE; y++)
                for (i64 x = -LOADED_CHUNK_DISTANCE; x <= LOADED_CHUNK_DISTANCE; x++)
                    add_missing_chunk(w, VEC3_ADD(((VEC3(i64)){ x, y, z }), w->old_chunk_pos));
        for (i64 z = -LOADED_CHUNK_DISTANCE + diff.z; z <= -LOADED_CHUNK_DISTANCE - 1; z++)
            for (i64 y = -LOADED_CHUNK_DISTANCE; y <= LOADED_CHUNK_DISTANCE; y++)
                for (i64 x = -LOADED_CHUNK_DISTANCE; x <= LOADED_CHUNK_DISTANCE; x++)
                    chunk_to_faces(*MAP_GET_T(ChunkPos, ChunkPtr, w->chunks,
                                              VEC3_ADD(((VEC3(i64)){ x, y, z }), w->old_chunk_pos)),
                                   &w->chunks, w->mapped_faces, &w->gpu_face_count);
    }
}

void generate_new_chunks(World* w, [[maybe_unused]] GLuint vao, size_t* face_count)
{
    TracyZone(ctx, "generate_chunks");

    size_t face_count_before = w->gpu_face_count;

    VEC3(i64)
    player_chunk_pos =
        (VEC3(i64)){ ((i64)w->player->pos.x / CHUNK_SIZE), ((i64)w->player->pos.y / CHUNK_SIZE),
                     ((i64)w->player->pos.z / CHUNK_SIZE) };

    VEC3(i64) diff = VEC3_SUB(player_chunk_pos, w->old_chunk_pos);

    Chunk** c = MAP_GET_T(ChunkPos, ChunkPtr, w->chunks, player_chunk_pos);

    if (c != NULL && diff.x == 0 && diff.y == 0 && diff.z == 0)
    {
        TracyZoneEnd(ctx);
        return;
    }

    if (!c)
    {
        TracyZone(ctx_init, "initial_gen");
        printf("Generating all surrounding chunks\n");
        for (i64 z = -LOADED_CHUNK_DISTANCE; z <= LOADED_CHUNK_DISTANCE; z++)
            for (i64 y = -LOADED_CHUNK_DISTANCE; y <= LOADED_CHUNK_DISTANCE; y++)
                for (i64 x = -LOADED_CHUNK_DISTANCE; x <= LOADED_CHUNK_DISTANCE; x++)
                    add_missing_chunk(w, VEC3_ADD(((VEC3(i64)){ x, y, z }), player_chunk_pos));
        for (i64 z = -LOADED_CHUNK_DISTANCE; z <= LOADED_CHUNK_DISTANCE; z++)
            for (i64 y = -LOADED_CHUNK_DISTANCE; y <= LOADED_CHUNK_DISTANCE; y++)
                for (i64 x = -LOADED_CHUNK_DISTANCE; x <= LOADED_CHUNK_DISTANCE; x++)
                    chunk_to_faces(*MAP_GET_T(ChunkPos, ChunkPtr, w->chunks,
                                              VEC3_ADD(((VEC3(i64)){ x, y, z }), player_chunk_pos)),
                                   &w->chunks, w->mapped_faces, &w->gpu_face_count);
        TracyZoneEnd(ctx_init);
    }
    else
    {
        TracyZone(ctx_border, "border_gen");
        generate_new_chunks_border(w, diff);
        TracyZoneEnd(ctx_border);
    }

    w->old_chunk_pos = player_chunk_pos;
    *face_count = w->gpu_face_count;

    TracyZone(ctx_upload, "face_upload");
    glBindBuffer(GL_ARRAY_BUFFER, w->face_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)(face_count_before * sizeof(Face)),
                    (GLsizeiptr)((w->gpu_face_count - face_count_before) * sizeof(Face)),
                    w->mapped_faces + face_count_before);
    TracyZoneEnd(ctx_upload);

    TracyZoneEnd(ctx);
}
