#pragma once

#include <glad/glad.h>
// glad before
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <stddef.h>
#include "utils/container.h"
#include "voxel/chunk.h"

typedef struct
{
    MAP(ChunkPos, ChunkPtr) chunks;
} World;

World init_world(void);
size_t regenerate_faces_buffer(World *w, GLuint vao);
