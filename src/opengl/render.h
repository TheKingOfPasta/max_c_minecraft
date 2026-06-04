#pragma once

#include <glad/glad.h>
// glad before
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include "voxel/world.h"

void render(GLuint fbo, GLuint prog, GLuint mvp_ubo, size_t face_count);
size_t regenerate_faces_buffer(World *w, GLuint vao);
