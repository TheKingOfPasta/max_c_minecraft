#include "mesh_worker.h"

#include <stdio.h>
#include <stdlib.h>

#include "opengl/tracy.h"
#include "utils/vec3.h"
#include "voxel/chunk.h"
#include "voxel/terrain_gen/gen.h"

static void add_chunk_if_missing(MeshWorker* mw, VEC3(i64) pos)
{
    if (MAP_GET_T(ChunkPos, ChunkPtr, mw->chunks, pos))
        return;
    Chunk* c = create_empty_chunk(pos);
    gen_terrain(42, c);
    MAP_INSERT_T(ChunkPos, ChunkPtr, mw->chunks, pos, c);
}

static void process_slab(MeshWorker* mw, VEC3(i64) from, VEC3(i64) to)
{
    TracyZone(ctx_slab, "process_slab");

    TracyZone(ctx_terrain, "terrain_gen");
    VEC3(i64) efrom = VEC3_SUB(from, VEC3_SPLAT(i64, 1));
    VEC3(i64) eto = VEC3_ADD(to, VEC3_SPLAT(i64, 1));
    for (i64 z = efrom.z; z <= eto.z; z++)
        for (i64 y = efrom.y; y <= eto.y; y++)
            for (i64 x = efrom.x; x <= eto.x; x++)
                add_chunk_if_missing(mw, (VEC3(i64)){ x, y, z });
    TracyZoneEnd(ctx_terrain);

    TracyZone(ctx_mesh, "mesh_slab");
    for (i64 z = from.z; z <= to.z; z++)
        for (i64 y = from.y; y <= to.y; y++)
            for (i64 x = from.x; x <= to.x; x++)
            {
                VEC3(i64) pos = { x, y, z };
                Chunk* c = *MAP_GET_T(ChunkPos, ChunkPtr, mw->chunks, pos);
                if (c->meshed)
                    continue;

                VECTOR(Face) faces;
                VECTOR_INIT(faces);
                chunk_to_faces(c, &mw->chunks, &faces);

                if (faces.size == 0)
                {
                    c->meshed = true;
                    VECTOR_FREE(faces);
                    continue;
                }

                TracyZone(ctx_push, "result_push");
                pthread_mutex_lock(&mw->res_mutex);
                if (!VECTOR_PUSH_BACK(mw->results, ((MWMeshResult){ c, faces })))
                {
                    fprintf(stderr, "mesh_worker: result queue OOM, dropping mesh\n");
                    VECTOR_FREE(faces);
                }
                else
                {
                    c->meshed = true;
                }
                pthread_mutex_unlock(&mw->res_mutex);
                TracyZoneEnd(ctx_push);
            }
    TracyZoneEnd(ctx_mesh);

    TracyZoneEnd(ctx_slab);
}

static void* worker_run(void* arg)
{
    TracySetThreadName("mesh_worker");
    MeshWorker* mw = arg;
    while (true)
    {
        pthread_mutex_lock(&mw->slab_mutex);
        while (mw->slab_jobs.size == 0 && !mw->stop)
            pthread_cond_wait(&mw->slab_cond, &mw->slab_mutex);
        if (mw->stop && mw->slab_jobs.size == 0)
        {
            pthread_mutex_unlock(&mw->slab_mutex);
            break;
        }
        MWSlabJob job = mw->slab_jobs.data[--mw->slab_jobs.size];
        pthread_mutex_unlock(&mw->slab_mutex);

        process_slab(mw, job.from, job.to);
    }
    return NULL;
}

MeshWorker* mesh_worker_create(void)
{
    MeshWorker* mw = calloc(1, sizeof(MeshWorker));
    pthread_mutex_init(&mw->slab_mutex, NULL);
    pthread_cond_init(&mw->slab_cond, NULL);
    pthread_mutex_init(&mw->res_mutex, NULL);
    VECTOR_INIT(mw->slab_jobs);
    VECTOR_INIT(mw->results);
    pthread_create(&mw->thread, NULL, worker_run, mw);
    return mw;
}

void mesh_worker_destroy(MeshWorker* mw)
{
    pthread_mutex_lock(&mw->slab_mutex);
    mw->stop = true;
    pthread_cond_signal(&mw->slab_cond);
    pthread_mutex_unlock(&mw->slab_mutex);
    pthread_join(mw->thread, NULL);
    free(mw);
}

void mesh_worker_submit(MeshWorker* mw, VEC3(i64) from, VEC3(i64) to)
{
    pthread_mutex_lock(&mw->slab_mutex);
    if (!VECTOR_PUSH_BACK(mw->slab_jobs, ((MWSlabJob){ from, to })))
        fprintf(stderr, "mesh_worker: slab queue OOM, dropping job\n");
    else
        pthread_cond_signal(&mw->slab_cond);
    pthread_mutex_unlock(&mw->slab_mutex);
}

bool mesh_worker_pop_result(MeshWorker* mw, MWMeshResult* out)
{
    pthread_mutex_lock(&mw->res_mutex);
    if (mw->results.size == 0)
    {
        pthread_mutex_unlock(&mw->res_mutex);
        return false;
    }
    *out = mw->results.data[--mw->results.size];
    pthread_mutex_unlock(&mw->res_mutex);
    return true;
}
