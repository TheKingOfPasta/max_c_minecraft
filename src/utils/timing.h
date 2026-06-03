#pragma once

#include <glad/glad.h>

#define ENABLE_PRINTS 0
#if ENABLE_PRINTS

typedef struct
{
    const char* name;
    double ms;
    int ran;
} TimingSlot;

#    define START_TIME(t)                                                                          \
        static TimingSlot _ts_##t = { .name = #t };                                                \
        static TimingSlot* _tsp_##t __attribute__((section("timing_ptrs"), used)) = &_ts_##t;      \
        GLuint _query_##t;                                                                         \
        glGenQueries(1, &_query_##t);                                                              \
        glBeginQuery(GL_TIME_ELAPSED, _query_##t);

#    define END_TIME(t)                                                                            \
        glEndQuery(GL_TIME_ELAPSED);                                                               \
        {                                                                                          \
            GLuint64 _e;                                                                           \
            glGetQueryObjectui64v(_query_##t, GL_QUERY_RESULT, &_e);                               \
            _ts_##t.ms = _e / 1e6;                                                                 \
            _ts_##t.ran = 1;                                                                       \
        }

#else
#    define START_TIME(t) ;
#    define END_TIME(t) ;
#endif
