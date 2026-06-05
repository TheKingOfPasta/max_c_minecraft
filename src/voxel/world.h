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

typedef struct
{
    GLuint count;
    GLuint instanceCount;
    GLuint first;
    GLuint baseInstance;
} DrawInstance;

VECTOR_DECLARE(DrawInstance);

typedef struct
{
    MAP(ChunkPos, ChunkPtr) chunks;
    Player* player;
    VEC3(i64) old_chunk_pos;
    VECTOR(DrawInstance) drawn_chunks;
} World;

World init_world(void);
void generate_new_chunks(World* w, GLuint vao, size_t* face_count);
