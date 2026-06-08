#pragma once

#include <pthread.h>
#include <stddef.h>

#include "utils/vec3.h"
#include "voxel/chunk.h"
#include "voxel/face.h"

typedef struct
{
    VEC3(i64) from;
    VEC3(i64) to;
} MWSlabJob;

typedef struct
{
    Chunk* chunk;
    VECTOR(Face) faces;
} MWMeshResult;

VECTOR_DECLARE(MWSlabJob);
VECTOR_DECLARE(MWMeshResult);

typedef struct MeshWorker
{
    VECTOR(MWSlabJob) slab_jobs;
    pthread_mutex_t slab_mutex;
    pthread_cond_t slab_cond;

    VECTOR(MWMeshResult) results;
    pthread_mutex_t res_mutex;

    MAP(ChunkPos, ChunkPtr) chunks;

    bool stop;
    pthread_t thread;
} MeshWorker;

MeshWorker* mesh_worker_create(void);
void mesh_worker_destroy(MeshWorker* mw);
void mesh_worker_submit(MeshWorker* mw, VEC3(i64) from, VEC3(i64) to);
bool mesh_worker_pop_result(MeshWorker* mw, MWMeshResult* out);
