#include "render.h"

#include "mvp_model.h"
#include "opengl/tracy.h"
#include "utils/container.h"
#include "utils/vec3.h"
#include "voxel/chunk.h"
#include "voxel/world.h"

typedef struct
{
    VEC3(float) n;
    float w;
} Plane;
typedef struct
{
    Plane p[6];
} Frustum;

static Frustum frustum_extract(const mat4 proj, const mat4 view)
{
    mat4 vp;
    mat4_mul(vp, proj, view);

    return (Frustum){ .p = {
                          { .n = { vp[0] + vp[3], vp[4] + vp[7], vp[8] + vp[11] },
                            .w = vp[12] + vp[15] }, // left
                          { .n = { vp[3] - vp[0], vp[7] - vp[4], vp[11] - vp[8] },
                            .w = vp[15] - vp[12] }, // right
                          { .n = { vp[1] + vp[3], vp[5] + vp[7], vp[9] + vp[11] },
                            .w = vp[13] + vp[15] }, // bottom
                          { .n = { vp[3] - vp[1], vp[7] - vp[5], vp[11] - vp[9] },
                            .w = vp[15] - vp[13] }, // top
                          { .n = { vp[2] + vp[3], vp[6] + vp[7], vp[10] + vp[11] },
                            .w = vp[14] + vp[15] }, // near
                          { .n = { vp[3] - vp[2], vp[7] - vp[6], vp[11] - vp[10] },
                            .w = vp[15] - vp[14] }, // far
                      } };
}

static bool aabb_in_frustum(const Frustum* f, VEC3(float) min, VEC3(float) max)
{
    for (int i = 0; i < 6; i++)
    {
        Plane p = f->p[i];
        VEC3(float)
        pv = {
            p.n.x >= 0 ? max.x : min.x,
            p.n.y >= 0 ? max.y : min.y,
            p.n.z >= 0 ? max.z : min.z,
        };
        if (VEC3_DOT(p.n, pv) + p.w < 0)
            return false;
    }
    return true;
}

static void create_draw_call_list(World* w)
{
    VECTOR_FREE(w->drawn_chunks);
    VECTOR_INIT(w->drawn_chunks);
    VECTOR_RESIZE(w->drawn_chunks, w->chunks.size / 3);

    Frustum f = frustum_extract(mvp->proj, mvp->view);

    MAP_FOR_EACH(w->chunks, IT)
    {
        ChunkPtr c = (*IT).value;

        if (c->face_count == 0)
            continue;

        VEC3(float) min = VEC3_CAST(float, VEC3_SCALE(c->pos, CHUNK_SIZE));
        VEC3(float) max = VEC3_ADD(min, ((VEC3(float)){ CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE }));

        if (aabb_in_frustum(&f, min, max))
            VECTOR_PUSH_BACK(w->drawn_chunks,
                             ((DrawInstance){ 6, c->face_count, 0, c->face_start_index }));
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

    TracyZone(ctx_cull, "frustum_cull");
    create_draw_call_list(w);
    TracyZoneEnd(ctx_cull);

    TracyZone(ctx_draw, "draw_instances");
    draw_instances(w, draw_instances_vbo);
    TracyZoneEnd(ctx_draw);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, WIN_W, WIN_H, 0, 0, WIN_W, WIN_H, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void draw_instances(World* w, GLuint draw_instances_vbo)
{
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw_instances_vbo);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(DrawInstance) * w->drawn_chunks.size,
                 w->drawn_chunks.data, GL_DYNAMIC_DRAW);

    glMultiDrawArraysIndirect(GL_TRIANGLES, 0, w->drawn_chunks.size, 0);
}
