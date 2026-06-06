#pragma once

#include <float.h>
#include <stdio.h>

#define BENCH_WINDOW 100

typedef struct
{
    float frames[BENCH_WINDOW];
    int head;
    int count;
    double accum;
} BenchStats;

typedef struct
{
    bool active;
} Bench;

static inline void bench_hud(BenchStats* s, float dt, int drawn, int total)
{
    s->frames[s->head] = dt;
    s->head = (s->head + 1) % BENCH_WINDOW;
    if (s->count < BENCH_WINDOW)
        s->count++;
    s->accum += dt;

    if (s->accum < 1.0)
        return;
    s->accum = 0.0;

    float sum = 0, mn = FLT_MAX, mx = 0;
    for (int i = 0; i < s->count; i++)
    {
        float f = s->frames[i];
        sum += f;
        if (f < mn)
            mn = f;
        if (f > mx)
            mx = f;
    }
    float avg = sum / (float)s->count;
    printf("\ravg %5.2f ms  min %5.2f ms  max %5.2f ms  fps %5.0f  chunks %d/%d          ",
           avg * 1000.f, mn * 1000.f, mx * 1000.f, 1.f / avg, drawn, total);
    fflush(stdout);
}
