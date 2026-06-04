#include "world.h"
#include "utils/container.h"
#include "voxel/chunk.h"

World init_world(void)
{
    World w = {
        MAP_INIT()
    };

    VECTOR_INIT(w.chunks);

    Chunk* c = create_random_chunk(0, 0, 0);

    ChunkPos pos = (VEC3(i64)){ 0, 0, 0 };

    MAP_INSERT_T(ChunkPos, ChunkPtr, w.chunks, pos, c);

    return w;
}
