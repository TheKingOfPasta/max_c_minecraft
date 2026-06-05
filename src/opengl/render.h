#pragma once

#include <glad/glad.h>
// glad before
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include "voxel/world.h"

void render(GLuint fbo, GLuint prog, GLuint mvp_ubo, World* w, GLuint vbo);

void draw_instances(World* w, GLuint vbo);
