#pragma once

#define VEC3(T)                                     \
    vec3_##T

#define DECLARE_VEC3(T)                             \
    typedef struct                                  \
    {                                               \
        T x;                                        \
        T y;                                        \
        T z;                                        \
    } VEC3(T);

#define VEC3_ADD(A, B)                              \
    ((typeof(A)){ .x = A.x+B.x, .y = A.y+B.y, .z = A.z+B.z })

#define VEC3_ADD_INPLACE(A, B)                      \
    {                                               \
        A.x += B.x;                                 \
        A.y += B.y;                                 \
        A.z += B.z;                                 \
    }

#define VEC3_SUB(A, B)                              \
    ((typeof(A)){ .x = A.x-B.x, .y = A.y-B.y, .z = A.z-B.z })

#define VEC3_SUB_INPLACE(A, B)                      \
    {                                               \
        A.x -= B.x;                                 \
        A.y -= B.y;                                 \
        A.z -= B.z;                                 \
    }

#define VEC3_MOD(A, K)                              \
    ((typeof(A)){ .x = A.x%B.x, .y = A.y%B.y, .z = A.z%B.z })

#define VEC3_DIV(A, B)                              \
    ((typeof(A)){ .x = A.x/B.x, .y = A.y/B.y, .z = A.z/B.z })

#define VEC3_MUL(A, K)                              \
    ((typeof(A)){ .x = A.x*B.x, .y = A.y*B.y, .z = A.z*B.z })

#define VEC3_DOT(A, B)                              \
    (A.x*B.x + A.y*B.y + A.z*B.z)

#define VEC3_FOR_EACH(arr, n, e)                        \
    for (int i__LINE__ = 0; i__LINE__ < n;) \
        for (typeof(*arr) e = arr[0]; i__LINE__ < n; i__LINE__++, e = arr[i__LINE__])

#define VEC3_CAST(A, OTHERTYPE)

#define VEC3_MAP(A, OTHERTYPE, e, body)
