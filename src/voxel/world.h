#pragma once

#include <glad/glad.h>
// glad before
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <stddef.h>

#include "utils/container.h"
#include "voxel/chunk.h"
#include "voxel/player.h"

#define LOADED_CHUNK_DISTANCE 20
#define MAX_FACE_COUNT ((size_t)1 << 26)

typedef struct
{
    GLuint count;
    GLuint instanceCount;
    GLuint first;
    GLuint baseInstance;
} DrawInstance;

VECTOR_DECLARE(DrawInstance);

typedef struct MeshWorker MeshWorker;

typedef struct
{
    MeshWorker* worker;
    bool initialized;
    Player* player;
    VEC3(i64) old_chunk_pos;
    VECTOR(ChunkPtr) meshed_chunks;
    VECTOR(DrawInstance) drawn_chunks;
    GLuint face_vbo;
    size_t gpu_face_count;
} World;

World init_world(void);
void world_init_gl(World* w, GLuint face_vbo);
void world_integrate_results(World* w);
void generate_new_chunks(World* w, GLuint vao, size_t* face_count);
