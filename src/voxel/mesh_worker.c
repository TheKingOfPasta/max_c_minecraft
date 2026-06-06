#include "mesh_worker.h"

#include <stdlib.h>

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
    // terrain gen: +1 chunk radius chunk gen so neighbours exist when meshing
    VEC3(i64) efrom = VEC3_SUB(from, VEC3_SPLAT(i64, 1));
    VEC3(i64) eto = VEC3_ADD(to, VEC3_SPLAT(i64, 1));
    for (i64 z = efrom.z; z <= eto.z; z++)
        for (i64 y = efrom.y; y <= eto.y; y++)
            for (i64 x = efrom.x; x <= eto.x; x++)
                add_chunk_if_missing(mw, (VEC3(i64)){ x, y, z });

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
                c->meshed = true;

                if (faces.size == 0)
                {
                    VECTOR_FREE(faces);
                    continue;
                }

                pthread_mutex_lock(&mw->res_mutex);
                int next = (mw->res_tail + 1) % RESULT_QUEUE_CAP;
                if (next != mw->res_head)
                {
                    mw->results[mw->res_tail] = (MWMeshResult){ c, faces };
                    mw->res_tail = next;
                }
                else
                {
                    VECTOR_FREE(faces);
                }
                pthread_mutex_unlock(&mw->res_mutex);
            }
}

static void* worker_run(void* arg)
{
    MeshWorker* mw = arg;
    while (true)
    {
        pthread_mutex_lock(&mw->slab_mutex);
        while (mw->slab_head == mw->slab_tail && !mw->stop)
            pthread_cond_wait(&mw->slab_cond, &mw->slab_mutex);
        if (mw->stop && mw->slab_head == mw->slab_tail)
        {
            pthread_mutex_unlock(&mw->slab_mutex);
            break;
        }
        MWSlabJob job = mw->slab_jobs[mw->slab_head];
        mw->slab_head = (mw->slab_head + 1) % SLAB_QUEUE_CAP;
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
    int next = (mw->slab_tail + 1) % SLAB_QUEUE_CAP;
    if (next != mw->slab_head)
    {
        mw->slab_jobs[mw->slab_tail] = (MWSlabJob){ from, to };
        mw->slab_tail = next;
        pthread_cond_signal(&mw->slab_cond);
    }
    pthread_mutex_unlock(&mw->slab_mutex);
}

bool mesh_worker_pop_result(MeshWorker* mw, MWMeshResult* out)
{
    pthread_mutex_lock(&mw->res_mutex);
    if (mw->res_head == mw->res_tail)
    {
        pthread_mutex_unlock(&mw->res_mutex);
        return false;
    }
    *out = mw->results[mw->res_head];
    mw->res_head = (mw->res_head + 1) % RESULT_QUEUE_CAP;
    pthread_mutex_unlock(&mw->res_mutex);
    return true;
}
