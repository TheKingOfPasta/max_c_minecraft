#include <glad/glad.h>
// glad before
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "opengl/instances.h"
#include "opengl/mvp_model.h"
#include "opengl/shader_compile.h"
#include "utils/container.h"
#include "utils/vec3.h"
#include "voxel/chunk.h"
#include "voxel/face.h"
#include "voxel/textures/array_texture.h"
#include "voxel/world.h"

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

static void render(GLuint fbo, GLuint prog, GLuint cube_vao, GLuint mvp_ubo, MAP(ChunkPos, ChunkPtr)* chunks)
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

    texture_array_init();

    GLuint mvp_ubo;
    bind_uniform_buffer(&mvp_ubo, BINDING_MVP, mvp, sizeof(mvp_model));

    GLuint instances_vbo = describe_faces(cube_vao);

    double last_t = glfwGetTime();

    World w = init_world();

    while (!glfwWindowShouldClose(win))
    {
        double t0 = glfwGetTime();
        float dt = (float)(t0 - last_t);
        last_t = t0;

        update_camera(win, &state, dt);
        mat4_view_from_camera(mvp->view, state.cam_pos, state.cam_pitch, state.cam_yaw);

        render(fbo, render_prog, cube_vao, mvp_ubo, &w.chunks);

        printf("\r%f                        ", 1.0 / dt);
        fflush(stdout);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glfwTerminate();
    free(mvp);
}
