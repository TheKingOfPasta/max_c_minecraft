#pragma once

#include "utils/container.h"
#include "utils/vec3.h"

typedef struct
{
    int texture_id;
    int face_id;
    VEC3(i32) pos;
} Face;

VECTOR_DECLARE(Face);
