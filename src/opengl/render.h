#pragma once

#include <glad/glad.h>
// glad before
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include "utils/container.h"
#include "voxel/chunk.h"

void render(GLuint fbo, GLuint prog, GLuint cube_vao, GLuint mvp_ubo, MAP(ChunkPos, ChunkPtr)* chunks);
