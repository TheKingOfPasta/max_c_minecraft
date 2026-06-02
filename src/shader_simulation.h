#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>

#include "utils/vec3.h"

typedef struct
{
    float dt;

    GLuint simulation_ubo;

    float padding;

    VEC3(float) cam_pos;
    float cam_pitch;
    float cam_yaw;
} shader_simulation;

extern shader_simulation *s;
