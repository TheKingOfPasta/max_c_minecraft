#include "world.h"

#include <glad/glad.h>
// glad before
#include <GL/gl.h>
#include <math.h>
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
    if (MAP_GET_T(ChunkPos, ChunkPtr, w->chunks, pos))
        return;

    Chunk* c = create_empty_chunk(pos);
    gen_terrain(42, c);
    MAP_INSERT_T(ChunkPos, ChunkPtr, w->chunks, pos, c);
}

static void generate_slab(World* w, VEC3(i64) from, VEC3(i64) to)
{
    for (i64 z = from.z; z <= to.z; z++)
        for (i64 y = from.y; y <= to.y; y++)
            for (i64 x = from.x; x <= to.x; x++)
                add_missing_chunk(w, (VEC3(i64)){ x, y, z });

    for (i64 z = from.z; z <= to.z; z++)
        for (i64 y = from.y; y <= to.y; y++)
            for (i64 x = from.x; x <= to.x; x++)
            {
                Chunk* c = *MAP_GET_T(ChunkPos, ChunkPtr, w->chunks, ((VEC3(i64)){ x, y, z }));
                if (!c->meshed)
                {
                    chunk_to_faces(c, &w->chunks, w->mapped_faces, &w->gpu_face_count);
                    c->meshed = true;
                }
            }
}

static void generate_new_chunks_border(World* w, VEC3(i64) ppos, VEC3(i64) diff)
{
    VEC3(i64) lo = VEC3_SUB(ppos, VEC3_SPLAT(i64, LOADED_CHUNK_DISTANCE));
    VEC3(i64) hi = VEC3_ADD(ppos, VEC3_SPLAT(i64, LOADED_CHUNK_DISTANCE));

    if (diff.x > 0)
        generate_slab(w, (VEC3(i64)){ ppos.x - diff.x + LOADED_CHUNK_DISTANCE + 1, lo.y, lo.z },
                      hi);
    else if (diff.x < 0)
        generate_slab(w, lo,
                      (VEC3(i64)){ ppos.x - diff.x - LOADED_CHUNK_DISTANCE - 1, hi.y, hi.z });

    if (diff.y > 0)
        generate_slab(w, (VEC3(i64)){ lo.x, ppos.y - diff.y + LOADED_CHUNK_DISTANCE + 1, lo.z },
                      hi);
    else if (diff.y < 0)
        generate_slab(w, lo,
                      (VEC3(i64)){ hi.x, ppos.y - diff.y - LOADED_CHUNK_DISTANCE - 1, hi.z });

    if (diff.z > 0)
        generate_slab(w, (VEC3(i64)){ lo.x, lo.y, ppos.z - diff.z + LOADED_CHUNK_DISTANCE + 1 },
                      hi);
    else if (diff.z < 0)
        generate_slab(w, lo,
                      (VEC3(i64)){ hi.x, hi.y, ppos.z - diff.z - LOADED_CHUNK_DISTANCE - 1 });
}

void generate_new_chunks(World* w, [[maybe_unused]] GLuint vao, size_t* face_count)
{
    TracyZone(ctx, "generate_chunks");

    size_t face_count_before = w->gpu_face_count;

    VEC3(i64)
    playerpos = {
        (i64)floorf(w->player->pos.x / CHUNK_SIZE),
        (i64)floorf(w->player->pos.y / CHUNK_SIZE),
        (i64)floorf(w->player->pos.z / CHUNK_SIZE),
    };

    VEC3(i64) diff = VEC3_SUB(playerpos, w->old_chunk_pos);

    Chunk** c = MAP_GET_T(ChunkPos, ChunkPtr, w->chunks, playerpos);

    if (c != NULL && diff.x == 0 && diff.y == 0 && diff.z == 0)
    {
        TracyZoneEnd(ctx);
        return;
    }

    if (!c)
    {
        TracyZone(ctx_init, "initial_gen");
        printf("Generating all surrounding chunks\n");
        VEC3(i64) lo = VEC3_SUB(playerpos, VEC3_SPLAT(i64, LOADED_CHUNK_DISTANCE));
        VEC3(i64) hi = VEC3_ADD(playerpos, VEC3_SPLAT(i64, LOADED_CHUNK_DISTANCE));
        generate_slab(w, lo, hi);
        TracyZoneEnd(ctx_init);
    }
    else
    {
        TracyZone(ctx_border, "border_gen");
        generate_new_chunks_border(w, playerpos, diff);
        TracyZoneEnd(ctx_border);
    }

    w->old_chunk_pos = playerpos;
    *face_count = w->gpu_face_count;

    TracyZone(ctx_upload, "face_upload");
    glBindBuffer(GL_ARRAY_BUFFER, w->face_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)(face_count_before * sizeof(Face)),
                    (GLsizeiptr)((w->gpu_face_count - face_count_before) * sizeof(Face)),
                    w->mapped_faces + face_count_before);
    TracyZoneEnd(ctx_upload);

    TracyZoneEnd(ctx);
}
