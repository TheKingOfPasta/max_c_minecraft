#include "mvp_model.h"

#include <math.h>
#include <string.h>

mvp_model* mvp = NULL;

void mat4_identity(mat4 m)
{
    memset(m, 0, sizeof(mat4));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

void mat4_perspective(mat4 m, float fov_y, float aspect, float znear, float zfar)
{
    memset(m, 0, sizeof(mat4));
    float f = 1.0f / tanf(fov_y * 0.5f);
    m[0] = f / aspect;
    m[5] = f;
    m[10] = -(zfar + znear) / (zfar - znear);
    m[11] = -1.0f;
    m[14] = -(2.0f * zfar * znear) / (zfar - znear);
}

void mat4_view_from_camera(mat4 m, float px, float py, float pz, float pitch, float yaw)
{
    float cyaw = cosf(-yaw), syaw = sinf(-yaw);
    float cpitch = cosf(pitch), spitch = sinf(pitch);

    float fx = -syaw * cpitch, fy = -spitch, fz = cyaw * cpitch;
    float rx = cyaw, ry = 0.0f, rz = syaw;
    float ux = -spitch * syaw, uy = cpitch, uz = spitch * cyaw;

    m[0] = rx;
    m[4] = ry;
    m[8] = rz;
    m[12] = -(rx * px + ry * py + rz * pz);
    m[1] = ux;
    m[5] = uy;
    m[9] = uz;
    m[13] = -(ux * px + uy * py + uz * pz);
    m[2] = -fx;
    m[6] = -fy;
    m[10] = -fz;
    m[14] = fx * px + fy * py + fz * pz;
    m[3] = 0;
    m[7] = 0;
    m[11] = 0;
    m[15] = 1;
}
