#ifdef TRACY_ENABLE

#    include "tracy.h"

#    include <cstring>
#    include <glad/glad.h>
#    include <tracy/Tracy.hpp>
#    include <tracy/TracyOpenGL.hpp>

namespace
{
    tracy::GpuCtxScope* zone_stack[16];
    int zone_depth = 0;
} // namespace

extern "C"
{
    void tracy_gl_init(void)
    {
        TracyGpuContext;
    }
    void tracy_gl_collect(void)
    {
        TracyGpuCollect;
    }

    void tracy_gl_zone_begin(const char* name)
    {
        const size_t n = strlen(name);
        zone_stack[zone_depth++] = new tracy::GpuCtxScope(0, name, n, name, n, name, n, true);
    }

    void tracy_gl_zone_end(void)
    {
        delete zone_stack[--zone_depth];
    }

} // extern "C"

#endif
