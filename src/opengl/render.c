#include "render.h"
#include <math.h>

#include "mvp_model.h"
#include "utils/container.h"
#include "voxel/chunk.h"
#include "voxel/world.h"

static void create_draw_call_list(World* w)
{
    VECTOR_FREE(w->drawn_chunks);
    VECTOR_INIT(w->drawn_chunks);
    VECTOR_RESIZE(w->drawn_chunks, w->chunks.size / 3);

    VEC3(float) cam_dir = (VEC3(float)){ sin(w->player->cam_yaw), -sin(w->player->cam_pitch), cos(w->player->cam_yaw) };

    MAP_FOR_EACH(w->chunks, IT)
    {
        ChunkPtr c = (*IT).value;

        if (c->face_count == 0)
            continue;

        VEC3(float) chunk_center = (VEC3(float)){ (0.5 + c->pos.x) * CHUNK_SIZE, (0.5 + c->pos.y) * CHUNK_SIZE, (0.5 + c->pos.z) * CHUNK_SIZE };
        VEC3(float) diff = VEC3_SUB(chunk_center, w->player->pos);
        float length = sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);

        float one_over_length = 1.0f / length;
        VEC3_SCALE_INPLACE(diff, one_over_length);

        if (length < CHUNK_SIZE * 4 || VEC3_EQ(w->old_chunk_pos, c->pos) || VEC3_DOT(cam_dir, diff) > -0.2)
        {
            VECTOR_PUSH_BACK(w->drawn_chunks, ((DrawInstance){ 6, c->face_count, 0, c->face_start_index }));
        }
    }
}

void render(GLuint fbo, GLuint prog, GLuint mvp_ubo, World* w, GLuint draw_instances_vbo)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, WIN_W, WIN_H);
    glClearColor(0.4f, 0.5f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(prog);
    glBindBuffer(GL_UNIFORM_BUFFER, mvp_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mvp_model), mvp);

    create_draw_call_list(w);
    draw_instances(w, draw_instances_vbo);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, WIN_W, WIN_H, 0, 0, WIN_W, WIN_H, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void draw_instances(World* w, GLuint draw_instances_vbo)
{
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw_instances_vbo);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(DrawInstance) * w->drawn_chunks.size, w->drawn_chunks.data, GL_DYNAMIC_DRAW);

    glMultiDrawArraysIndirect(GL_TRIANGLES, 0, w->drawn_chunks.size, 0);
}
