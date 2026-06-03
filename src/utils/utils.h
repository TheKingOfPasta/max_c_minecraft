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

uint32_t clamp(uint32_t value, uint32_t min, uint32_t max);
