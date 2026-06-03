#pragma once

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

void mat4_identity(mat4 m);
void mat4_perspective(mat4 m, float fov_y, float aspect, float znear, float zfar);
void mat4_view_from_camera(mat4 m, float px, float py, float pz, float pitch, float yaw);
