#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>

#include "utils/vec3.h"

// Don't change order, padding matters
typedef struct
{
    float dt;

    GLuint simulation_ubo;

    float cam_pitch;
    float cam_yaw;

    VEC3(float) cam_pos;
} shader_simulation;

extern shader_simulation *s;
