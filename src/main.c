#include <glad/glad.h>
// glad before
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "opengl/mvp_model.h"
#include "opengl/shader_compile.h"
#include "utils/vec3.h"
#include "voxel/textures/array_texture.h"

#if defined(__NIXOS__)
#    define WIN_W 1920
#    define WIN_H 1200
#else
#    define WIN_W 1920
#    define WIN_H 1080
#endif

static void init_mvp(void)
{
    mvp = calloc(1, sizeof(mvp_model));
    mat4_identity(mvp->model);
    mat4_identity(mvp->view);
    mat4_perspective(mvp->proj, FOV_Y, (float)WIN_W / WIN_H, 0.1f, 1000.0f);
}

static GLuint create_cube_vao(void)
{
    // clang-format off
    /*float vertices[] = {
        // front
        -0.5f, -0.5f, 0.5f,  0.5f, -0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f,  0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        // back
        0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f,
        0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f,  0.5f, 0.5f, -0.5f,
        // left
        -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f,
        // right
        0.5f, -0.5f, 0.5f,  0.5f, -0.5f, -0.5f,  0.5f, 0.5f, -0.5f,
        0.5f, -0.5f, 0.5f,  0.5f, 0.5f, -0.5f,  0.5f, 0.5f, 0.5f,
        // top
        -0.5f, 0.5f, 0.5f,  0.5f, 0.5f, 0.5f,  0.5f, 0.5f, -0.5f,
        -0.5f, 0.5f, 0.5f,  0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f,
        // bottom
        -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, 0.5f,
        -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f,
    };*/
    // clang-format on

    GLuint vao/*, vbo*/;
    glGenVertexArrays(1, &vao);
    //glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    //glBindBuffer(GL_ARRAY_BUFFER, vbo);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //glEnableVertexAttribArray(LOCATION_POS);
    //glVertexAttribPointer(LOCATION_POS, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
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

typedef struct
{
    int face;
    VEC3(i32) position;
} block_face;

#define INSTANCE_COUNT 6

static void render(GLuint fbo, GLuint prog, GLuint cube_vao, GLuint mvp_ubo)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, WIN_W, WIN_H);
    glClearColor(0.4f, 0.5f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(prog);
    glBindBuffer(GL_UNIFORM_BUFFER, mvp_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mvp_model), mvp);

    glBindVertexArray(cube_vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, INSTANCE_COUNT);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, WIN_W, WIN_H, 0, 0, WIN_W, WIN_H, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main(void)
{
    srand(time(NULL));

    init_mvp();

    AppState state = {
        .mouse_initialized = false,
        .cam_pitch = 0.0f,
        .cam_yaw = 0.0f,
        .cam_pos = { .x = 0, .y = 0, .z = -3 },
    };

    GLFWwindow* win = init_window(&state);

    GLuint render_prog =
        create_program(compile_shader(GL_VERTEX_SHADER, "src/shaders/shader.vert"),
                       compile_shader(GL_FRAGMENT_SHADER, "src/shaders/shader.frag"));

    GLuint cube_vao = create_cube_vao();
    GLuint fbo = create_fbo(WIN_W, WIN_H);

    GLuint tex_array = load_block_texture_array();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex_array);

    glEnable(GL_DEPTH_TEST);

    GLuint mvp_ubo;
    bind_uniform_buffer(&mvp_ubo, BINDING_MVP, mvp, sizeof(mvp_model));

    double last_t = glfwGetTime();

    block_face instances[INSTANCE_COUNT] = {
        (block_face){ 0, (VEC3(i32)){ 0, 0, 0 } },
        (block_face){ 1, (VEC3(i32)){ 0, 0, 0 } },
        (block_face){ 2, (VEC3(i32)){ 0, 0, 0 } },
        (block_face){ 3, (VEC3(i32)){ 0, 0, 0 } },
        (block_face){ 4, (VEC3(i32)){ 0, 0, 0 } },
        (block_face){ 5, (VEC3(i32)){ 0, 0, 0 } },
    };

    GLuint instances_vbo;
    glGenBuffers(1, &instances_vbo);

    glBindVertexArray(cube_vao);
    glBindBuffer(GL_ARRAY_BUFFER, instances_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(block_face) * INSTANCE_COUNT, instances, GL_STATIC_DRAW);

    glVertexAttribIPointer(0, 1, GL_INT,   sizeof(block_face), (void*)offsetof(block_face, face));
    glEnableVertexAttribArray(0);
    glVertexAttribDivisor(0, 1);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(block_face), (void*)offsetof(block_face, position));
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    while (!glfwWindowShouldClose(win))
    {
        double t0 = glfwGetTime();
        float dt = (float)(t0 - last_t);
        last_t = t0;

        update_camera(win, &state, dt);
        mat4_view_from_camera(mvp->view, state.cam_pos, state.cam_pitch, state.cam_yaw);

        render(fbo, render_prog, cube_vao, mvp_ubo);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glfwTerminate();
    free(mvp);
}
