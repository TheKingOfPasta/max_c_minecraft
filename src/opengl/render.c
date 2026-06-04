#include "render.h"

#include "mvp_model.h"
#include "opengl/instances.h"
#include "voxel/world.h"

void render(GLuint fbo, GLuint prog, GLuint mvp_ubo, size_t face_count)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, WIN_W, WIN_H);
    glClearColor(0.4f, 0.5f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(prog);
    glBindBuffer(GL_UNIFORM_BUFFER, mvp_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mvp_model), mvp);

    draw_instances(face_count);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, WIN_W, WIN_H, 0, 0, WIN_W, WIN_H, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
