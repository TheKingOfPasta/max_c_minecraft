#include <glad/glad.h>
// glad before
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "inputs/inputs.h"
#include "opengl/instances.h"
#include "opengl/mvp_model.h"
#include "opengl/render.h"
#include "opengl/shader_compile.h"
#include "opengl/tracy.h"
#include "utils/bench.h"
#include "voxel/textures/array_texture.h"
#include "voxel/world.h"

static void init_mvp(void)
{
    mvp = calloc(1, sizeof(mvp_model));
    mat4_identity(mvp->model);
    mat4_identity(mvp->view);
    mat4_perspective(mvp->proj, FOV_Y, (float)WIN_W / WIN_H, 0.1f, 1000.0f);
}

static GLuint create_cube_vao(void)
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    return vao;
}

static GLuint create_fbo(int w, int h)
{
    GLuint fbo, tex, depth;

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    glGenRenderbuffers(1, &depth);
    glBindRenderbuffer(GL_RENDERBUFFER, depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    return fbo;
}

int main(int argc, char** argv)
{
    srand(time(NULL));

    Bench bench = bench_parse_args(argc, argv);

    init_mvp();

    AppState state = { .mouse_initialized = false };
    GLFWwindow* win = init_window(&state);

    GLuint render_prog =
        create_program(compile_shader(GL_VERTEX_SHADER, "src/shaders/shader.vert"),
                       compile_shader(GL_FRAGMENT_SHADER, "src/shaders/shader.frag"));

    GLuint vao = create_cube_vao();
    GLuint fbo = create_fbo(WIN_W, WIN_H);

    texture_array_init();

    GLuint mvp_ubo;
    bind_uniform_buffer(&mvp_ubo, BINDING_MVP, mvp, sizeof(mvp_model));
    describe_faces(vao);

    double last_t = glfwGetTime();

    World w = init_world();
    state.w = &w;

    size_t face_count;
    generate_new_chunks(&w, vao, &face_count);

    GLuint draw_instances_vbo;
    glGenBuffers(1, &draw_instances_vbo);

    TracyGlInit();

    BenchStats hud = { 0 };

    while (!glfwWindowShouldClose(win))
    {
        double t0 = glfwGetTime();
        float dt = (float)(t0 - last_t);
        last_t = t0;

        TracyZone(ctx_input, "input");
        if (bench.active)
        {
            bench_update(&bench, &w, dt);
            if (bench.phase == BENCH_PHASE_DONE)
            {
                TracyZoneEnd(ctx_input);
                break;
            }
        }
        else
            update_camera(win, &state, dt);
        TracyZoneEnd(ctx_input);

        mat4_view_from_camera(mvp->view, w.player->pos, w.player->cam_pitch, w.player->cam_yaw);

        TracyZone(ctx_chunks, "chunk_gen");
        generate_new_chunks(&w, vao, &face_count);
        TracyZoneEnd(ctx_chunks);

        TracyGlZone("render");
        render(fbo, render_prog, mvp_ubo, &w, draw_instances_vbo);
        TracyGlZoneEnd();
        TracyGlCollect(); // mark one frame

        bench_hud(&hud, dt, (int)w.drawn_chunks.size, (int)w.chunks.size);
        TRACY_FRAME_MARK;

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glfwTerminate();
    free(mvp);
}
