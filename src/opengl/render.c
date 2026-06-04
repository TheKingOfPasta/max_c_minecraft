#include "render.h"

#include "mvp_model.h"
#include "opengl/instances.h"

void render(GLuint fbo, GLuint prog, GLuint cube_vao, GLuint mvp_ubo, MAP(ChunkPos, ChunkPtr)* chunks)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, WIN_W, WIN_H);
    glClearColor(0.4f, 0.5f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(prog);
    glBindBuffer(GL_UNIFORM_BUFFER, mvp_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mvp_model), mvp);

    glBindVertexArray(cube_vao);

    VECTOR(Face) face_instances;
    VECTOR_INIT(face_instances);

    MAP_FOR_EACH((*chunks), c)
    {
        face_instances = chunk_to_faces(c->value, face_instances);
    }

    draw_instances(face_instances);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, WIN_W, WIN_H, 0, 0, WIN_W, WIN_H, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
