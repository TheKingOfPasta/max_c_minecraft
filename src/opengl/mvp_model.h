#pragma once

#include "utils/vec3.h"

typedef float mat4[16];

typedef struct
{
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp_model;

extern mvp_model* mvp;

#define BINDING_MVP 0
#define LOCATION_POS 0
#define FOV_Y (3.14159265f / 2.0f)

void mat4_identity(mat4 m);
void mat4_perspective(mat4 m, float fov_y, float aspect, float znear, float zfar);
void mat4_view_from_camera(mat4 m, VEC3(float) pos, float pitch, float yaw);
