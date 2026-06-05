#pragma once

#include "utils/vec3.h"

typedef struct
{
    VEC3(float) pos;
    float cam_pitch;
    float cam_yaw;
} Player;
