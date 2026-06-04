#pragma once

#include <signal.h>
#include <stdint.h>
#include <stdio.h>

#define ASSERT(CONDITION, FORMAT, ...)                                                             \
    do                                                                                             \
    {                                                                                              \
        if (!(CONDITION))                                                                          \
        {                                                                                          \
            fprintf(stderr, "%s:%d -> %s :\n\t" FORMAT "\n", __FILE_NAME__, __LINE__, __func__,    \
                    ##__VA_ARGS__);                                                                \
            raise(SIGABRT);                                                                        \
        }                                                                                          \
    } while (0);

#define COUNTOF(arr) sizeof((arr)) / sizeof(*(arr))

#define CLAMP(v, lo, hi)                                                                           \
    ({                                                                                             \
        auto _v = (v);                                                                             \
        auto _lo = (lo);                                                                           \
        auto _hi = (hi);                                                                           \
        _v < _lo ? _lo : (_v > _hi ? _hi : _v);                                                    \
    })

#define CLAMP01(v)                                                                                 \
    ({                                                                                             \
        auto _v = (v);                                                                             \
        _v < 0 ? 0 : (_v > 1 ? 1 : _v);                                                            \
    })

#define SMOOTHLERP(a, b, t)                                                                        \
    ({                                                                                             \
        auto _t = (t);                                                                             \
        auto _a = (a);                                                                             \
        auto _b = (b);                                                                             \
        _a + (_b - _a) * (_t * _t * (3.0f - 2.0f * _t));                                           \
    })
