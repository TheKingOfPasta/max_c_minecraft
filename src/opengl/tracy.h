#pragma once

#ifdef TRACY_ENABLE
#    include <tracy/TracyC.h>

#    define TracyZone(ctx, name) TracyCZoneN(ctx, name, 1)
#    define TracyZoneEnd(ctx) TracyCZoneEnd(ctx)
#    define TracySetThreadName(name) TracyCSetThreadName(name)
#    define TRACY_FRAME_MARK TracyCFrameMark
#    define TracyGlInit() tracy_gl_init()
#    define TracyGlZone(name) tracy_gl_zone_begin(name)
#    define TracyGlZoneEnd() tracy_gl_zone_end()
#    define TracyGlCollect() tracy_gl_collect()

#    ifdef __cplusplus
extern "C"
{
#    endif
    void tracy_gl_init(void);
    void tracy_gl_collect(void);
    void tracy_gl_zone_begin(const char* name);
    void tracy_gl_zone_end(void);
#    ifdef __cplusplus
}
#    endif

#else
#    define TracyZone(ctx, name)
#    define TracyZoneEnd(ctx)
#    define TracySetThreadName(name)
#    define TRACY_FRAME_MARK
#    define TracyGlInit()
#    define TracyGlZone(name)
#    define TracyGlZoneEnd()
#    define TracyGlCollect()
#endif
