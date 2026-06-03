#pragma once

#include <glad/glad.h>
// glad before
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <inputs/inputs.h>

GLuint compile_shader(GLenum type, char* file_name);
GLuint create_program(GLuint s1, GLuint s2);
void bind_uniform_buffer(GLuint* ubo, GLuint binding, void* ptr, size_t elt_size);
GLFWwindow* init_window(AppState* state);
