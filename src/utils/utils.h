#pragma once

#include <glad/glad.h>
// glad
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include "shader_simulation.h"

char* read_all_file(char *file);

void opengl_add_config(shader_simulation* s);
GLuint opengl_add_array(void* array, int size, int index);
void opengl_prepare_program(GLuint program, shader_simulation* s);
void opengl_launch_last_prepared_program(size_t elt_count);
void opengl_launch_program(GLuint program, shader_simulation* s, size_t elt_count);
void bind_uniform_buffer(GLuint* ubo, GLuint binding, void* ptr, size_t elt_size);

GLuint create_compute_program(GLuint s, const char* src);
GLuint compile_shader(GLenum type, char* file_name);
GLuint create_program(GLuint s1, GLuint s2);
char* read_shader_includes(char* file);
char* read_shader(char* file);
GLFWwindow* init_window();

#define ENABLE_PRINTS false

#if ENABLE_PRINTS == true

#    define START_TIME(t)                                                                          \
        static TimingSlot _ts_##t = { .name = #t };                                                \
        static TimingSlot* _tsp_##t __attribute__((section("timing_ptrs"), used)) = &_ts_##t;      \
        GLuint _query_##t;                                                                         \
        glGenQueries(1, &_query_##t);                                                              \
        glBeginQuery(GL_TIME_ELAPSED, _query_##t);

#    define END_TIME(t)                                                                            \
        glEndQuery(GL_TIME_ELAPSED);                                                               \
        {                                                                                          \
            GLuint64 _e;                                                                           \
            glGetQueryObjectui64v(_query_##t, GL_QUERY_RESULT, &_e);                               \
            _ts_##t.ms = _e / 1e6;                                                                 \
            _ts_##t.ran = 1;                                                                       \
        }

#else
#    define START_TIME(t) ;
#    define END_TIME(t) ;
#endif
