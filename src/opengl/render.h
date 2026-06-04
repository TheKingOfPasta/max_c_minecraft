#pragma once

#include <glad/glad.h>
// glad before
#include <GL/gl.h>
#include <GLFW/glfw3.h>

void render(GLuint fbo, GLuint prog, GLuint mvp_ubo, size_t face_count);
