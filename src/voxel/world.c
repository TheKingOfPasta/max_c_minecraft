#include "world.h"

#include <glad/glad.h>
// glad before
#include <GL/gl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "opengl/tracy.h"
#include "utils/vec3.h"
#include "voxel/face.h"
#include "voxel/generation/mesh_worker.h"

World init_world(void)
{
    World w = {
        .player = calloc(1, sizeof(Player)),
        .worker = mesh_worker_create(),
    };
    VECTOR_INIT(w.meshed_chunks);
    VECTOR_INIT(w.drawn_chunks);
    return w;
}

void world_init_gl(World* w, GLuint face_vbo)
{
    w->face_vbo = face_vbo;
    glBindBuffer(GL_ARRAY_BUFFER, face_vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_FACE_COUNT * sizeof(Face), NULL, GL_DYNAMIC_DRAW);
}

void world_integrate_results(World* w)
{
    MWMeshResult r;
    if (!mesh_worker_pop_result(w->worker, &r))
        return;

    TracyZone(ctx, "face_upload");
    glBindBuffer(GL_ARRAY_BUFFER, w->face_vbo);
    do
    {
        if (w->gpu_face_count + r.faces.size > MAX_FACE_COUNT)
        {
            fprintf(stderr, "face buffer full: %zu / %zu faces used\n", w->gpu_face_count,
                    MAX_FACE_COUNT);
            VECTOR_FREE(r.faces);
            continue;
        }
        r.chunk->face_start_index = w->gpu_face_count;
        r.chunk->face_count = r.faces.size;
        glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)(w->gpu_face_count * sizeof(Face)),
                        (GLsizeiptr)(r.faces.size * sizeof(Face)), r.faces.data);
        w->gpu_face_count += r.faces.size;
        VECTOR_PUSH_BACK(w->meshed_chunks, r.chunk);
        VECTOR_FREE(r.faces);
    } while (mesh_worker_pop_result(w->worker, &r));
    TracyZoneEnd(ctx);
}

static void submit_border(World* w, VEC3(i32) ppos, VEC3(i32) diff)
{
    VEC3(i32) lo = VEC3_SUB(ppos, VEC3_SPLAT(i32, LOADED_CHUNK_DISTANCE));
    VEC3(i32) hi = VEC3_ADD(ppos, VEC3_SPLAT(i32, LOADED_CHUNK_DISTANCE));

    if (diff.x > 0)
        mesh_worker_submit(
            w->worker, (VEC3(i32)){ ppos.x - diff.x + LOADED_CHUNK_DISTANCE + 1, lo.y, lo.z }, hi);
    else if (diff.x < 0)
        mesh_worker_submit(w->worker, lo,
                           (VEC3(i32)){ ppos.x - diff.x - LOADED_CHUNK_DISTANCE - 1, hi.y, hi.z });

    if (diff.y > 0)
        mesh_worker_submit(
            w->worker, (VEC3(i32)){ lo.x, ppos.y - diff.y + LOADED_CHUNK_DISTANCE + 1, lo.z }, hi);
    else if (diff.y < 0)
        mesh_worker_submit(w->worker, lo,
                           (VEC3(i32)){ hi.x, ppos.y - diff.y - LOADED_CHUNK_DISTANCE - 1, hi.z });

    if (diff.z > 0)
        mesh_worker_submit(
            w->worker, (VEC3(i32)){ lo.x, lo.y, ppos.z - diff.z + LOADED_CHUNK_DISTANCE + 1 }, hi);
    else if (diff.z < 0)
        mesh_worker_submit(w->worker, lo,
                           (VEC3(i32)){ hi.x, hi.y, ppos.z - diff.z - LOADED_CHUNK_DISTANCE - 1 });
}

void generate_new_chunks(World* w, [[maybe_unused]] GLuint vao, size_t* face_count)
{
    TracyZone(ctx, "generate_chunks");

    VEC3(i32)
    playerpos = {
        (i32)floorf(w->player->pos.x / CHUNK_SIZE),
        (i32)floorf(w->player->pos.y / CHUNK_SIZE),
        (i32)floorf(w->player->pos.z / CHUNK_SIZE),
    };

    VEC3(i32) diff = VEC3_SUB(playerpos, w->old_chunk_pos);

    if (w->initialized && diff.x == 0 && diff.y == 0 && diff.z == 0)
    {
        TracyZoneEnd(ctx);
        return;
    }

    if (!w->initialized)
    {
        TracyZone(ctx_init, "initial_gen");
        printf("Generating all surrounding chunks\n");
        VEC3(i32) lo = VEC3_SUB(playerpos, VEC3_SPLAT(i32, LOADED_CHUNK_DISTANCE));
        VEC3(i32) hi = VEC3_ADD(playerpos, VEC3_SPLAT(i32, LOADED_CHUNK_DISTANCE));
        mesh_worker_submit(w->worker, lo, hi);
        w->initialized = true;
        TracyZoneEnd(ctx_init);
    }
    else
    {
        TracyZone(ctx_border, "border_gen");
        submit_border(w, playerpos, diff);
        TracyZoneEnd(ctx_border);
    }

    w->old_chunk_pos = playerpos;
    *face_count = w->gpu_face_count;

    TracyZoneEnd(ctx);
}
