#include "noise.h"

#include <math.h>

#include "utils/type.h"

static inline float slerp(float a, float b, float t)
{
    return a + (b - a) * (t * t * (3.0f - 2.0f * t));
}

static u32 hash2(i64 x, i64 z, i64 seed)
{
    u64 h = (u64)(x * 1619LL + z * 31337LL + seed * 6971LL);
    h ^= h >> 17;
    h *= 0xbf58476d1ce4e5b9ULL;
    h ^= h >> 31;
    return (u32)h;
}

static u32 hash3(i64 x, i64 y, i64 z, i64 seed)
{
    u64 h = (u64)(x * 1619LL + y * 6451LL + z * 31337LL + seed * 6971LL);
    h ^= h >> 17;
    h *= 0xbf58476d1ce4e5b9ULL;
    h ^= h >> 31;
    return (u32)h;
}

static float gv2(i64 x, i64 z, i64 seed)
{
    return (float)(hash2(x, z, seed) >> 17) / (float)(1 << 15);
}

static float gv3(i64 x, i64 y, i64 z, i64 seed)
{
    return (float)(hash3(x, y, z, seed) >> 17) / (float)(1 << 15);
}

float noise2d(i64 seed, VEC3(float) pos, float scale, float amplitude)
{
    VEC3_DIV_INPLACE(pos, scale);
    VEC3(i64) i = { (i64)floorf(pos.x), (i64)floorf(pos.y), (i64)floorf(pos.z) };
    float fx = pos.x - (float)i.x;
    float fz = pos.z - (float)i.z;
    return amplitude
        * slerp(slerp(gv2(i.x, i.z, seed), gv2(i.x + 1, i.z, seed), fx),
                slerp(gv2(i.x, i.z + 1, seed), gv2(i.x + 1, i.z + 1, seed), fx), fz);
}

float noise3d(i64 seed, VEC3(float) pos, float scale, float amplitude)
{
    VEC3_DIV_INPLACE(pos, scale);
    VEC3(i64) i = { (i64)floorf(pos.x), (i64)floorf(pos.y), (i64)floorf(pos.z) };
    VEC3(float) f = { pos.x - (float)i.x, pos.y - (float)i.y, pos.z - (float)i.z };

    float vx00 = slerp(gv3(i.x, i.y, i.z, seed), gv3(i.x + 1, i.y, i.z, seed), f.x);
    float vx10 = slerp(gv3(i.x, i.y + 1, i.z, seed), gv3(i.x + 1, i.y + 1, i.z, seed), f.x);
    float vx01 = slerp(gv3(i.x, i.y, i.z + 1, seed), gv3(i.x + 1, i.y, i.z + 1, seed), f.x);
    float vx11 = slerp(gv3(i.x, i.y + 1, i.z + 1, seed), gv3(i.x + 1, i.y + 1, i.z + 1, seed), f.x);

    return amplitude * slerp(slerp(vx00, vx10, f.y), slerp(vx01, vx11, f.y), f.z);
}
