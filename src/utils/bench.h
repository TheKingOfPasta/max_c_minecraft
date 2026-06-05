#pragma once

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inputs/inputs.h"
#include "voxel/world.h"

#define BENCH_WINDOW 100

typedef struct
{
    float frames[BENCH_WINDOW];
    int head;
    int count;
    double accum;
} BenchStats;

typedef enum
{
    BENCH_PHASE_MOVE,
    BENCH_PHASE_IDLE,
    BENCH_PHASE_DONE
} BenchPhase;

typedef struct
{
    bool active;
    float phase_dur[2];
    float elapsed;
    BenchPhase phase;
} Bench;

static inline void bench_stats_push(BenchStats* s, float dt)
{
    s->frames[s->head] = dt;
    s->head = (s->head + 1) % BENCH_WINDOW;
    if (s->count < BENCH_WINDOW)
        s->count++;
    s->accum += dt;
}

static inline float bench_stats_avg(const BenchStats* s, float* out_mn, float* out_mx)
{
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
    *out_mn = mn;
    *out_mx = mx;
    return sum / (float)s->count;
}

static inline void bench_hud(BenchStats* s, float dt, int drawn, int total)
{
    bench_stats_push(s, dt);
    if (s->accum < 1.0)
        return;
    s->accum = 0.0;

    float mn, mx, avg = bench_stats_avg(s, &mn, &mx);
    printf("\ravg %5.2f ms  min %5.2f ms  max %5.2f ms  fps %5.0f  chunks %d/%d          ",
           avg * 1000.f, mn * 1000.f, mx * 1000.f, 1.f / avg, drawn, total);
    fflush(stdout);
}

static inline Bench bench_parse_args(int argc, char** argv)
{
    Bench bench = { .active = false };
    for (int i = 1; i + 2 < argc; i++)
        if (strcmp(argv[i], "--bench") == 0)
        {
            bench = (Bench){ .active = true,
                             .phase_dur = { (float)atof(argv[i + 1]), (float)atof(argv[i + 2]) },
                             .phase = BENCH_PHASE_MOVE };
            break;
        }
    return bench;
}

static inline void bench_update(Bench* bench, World* w, float dt)
{
    if (bench->phase == BENCH_PHASE_DONE)
        return;

    if (bench->phase == BENCH_PHASE_MOVE)
        w->player->pos.z -= CAM_SPEED * dt;

    bench->elapsed += dt;
    if (bench->elapsed >= bench->phase_dur[bench->phase])
    {
        bench->phase++;
        bench->elapsed = 0;
    }
}
