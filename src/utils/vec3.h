#pragma once

#include "type.h"

#define VEC3(T) vec3_##T

#define VEC3_DECLARE(T)                                                                            \
    typedef struct                                                                                 \
    {                                                                                              \
        T x;                                                                                       \
        T y;                                                                                       \
        T z;                                                                                       \
    } VEC3(T)

#define VEC3_SPLAT(T, V) ((VEC3(T)){ (V), (V), (V) })

#define VEC3_ADD(A, B) ((typeof(A)){ (A).x + (B).x, (A).y + (B).y, (A).z + (B).z })
#define VEC3_SUB(A, B) ((typeof(A)){ (A).x - (B).x, (A).y - (B).y, (A).z - (B).z })
#define VEC3_MOD(A, K) ((typeof(A)){ (A).x % (K), (A).y % (K), (A).z % (K) })
#define VEC3_DIV(A, K) ((typeof(A)){ (A).x / (K), (A).y / (K), (A).z / (K) })
#define VEC3_SCALE(A, K) ((typeof(A)){ (A).x * (K), (A).y * (K), (A).z * (K) })

#define VEC3_ADD_INPLACE(A, B)                                                                     \
    do                                                                                             \
    {                                                                                              \
        (A).x += (B).x;                                                                            \
        (A).y += (B).y;                                                                            \
        (A).z += (B).z;                                                                            \
    } while (0)
#define VEC3_SUB_INPLACE(A, B)                                                                     \
    do                                                                                             \
    {                                                                                              \
        (A).x -= (B).x;                                                                            \
        (A).y -= (B).y;                                                                            \
        (A).z -= (B).z;                                                                            \
    } while (0)

#define VEC3_DIV_INPLACE(A, K)                                                                     \
    do                                                                                             \
    {                                                                                              \
        (A).x /= (K);                                                                              \
        (A).y /= (K);                                                                              \
        (A).z /= (K);                                                                              \
    } while (0)
#define VEC3_SCALE_INPLACE(A, K)                                                                   \
    do                                                                                             \
    {                                                                                              \
        (A).x *= (K);                                                                              \
        (A).y *= (K);                                                                              \
        (A).z *= (K);                                                                              \
    } while (0)

#define VEC3_DOT(A, B) ((A).x * (B).x + (A).y * (B).y + (A).z * (B).z)
#define VEC3_EQ(A, B) ((A).x == (B).x && (A).y == (B).y && (A).z == (B).z)

#define VEC3_NORM(A)                                                                               \
    ({                                                                                             \
        typeof(A) _v = (A);                                                                        \
        float _l = sqrtf(_v.x * _v.x + _v.y * _v.y + _v.z * _v.z);                                 \
        (typeof(A)){ _v.x / _l, _v.y / _l, _v.z / _l };                                            \
    })

#define VEC3_CAST(T, V) ((VEC3(T)){ .x = (T)((V).x), .y = (T)((V).y), .z = (T)((V).z) })

VEC3_DECLARE(float);
VEC3_DECLARE(double);

VEC3_DECLARE(i8);
VEC3_DECLARE(i16);
VEC3_DECLARE(i32);
VEC3_DECLARE(i64);
VEC3_DECLARE(u8);
VEC3_DECLARE(u16);
VEC3_DECLARE(u32);
VEC3_DECLARE(u64);
