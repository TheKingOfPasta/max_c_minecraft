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

size_t regenerate_faces_buffer(World *w, GLuint vao)
{
    glBindVertexArray(vao);

    VECTOR(Face) face_instances;
    VECTOR_INIT(face_instances);

    MAP_FOR_EACH(w->chunks, c)
    {
        face_instances = chunk_to_faces(c->value, face_instances);
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(Face) * VECTOR_SIZE(face_instances), face_instances.data, GL_STATIC_DRAW);

    return face_instances.size;
}
