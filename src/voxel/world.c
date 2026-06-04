#include "world.h"
#include "utils/container.h"

World init_world(void)
{
    World w = {
        MAP_INIT()
    };

    return w;
}
