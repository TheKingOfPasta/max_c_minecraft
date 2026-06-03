#include <glad/glad.h>
// glad before
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "opengl/mvp_model.h"
#include "opengl/shader_compile.h"
#include "utils/vec3.h"

#if defined(__NIXOS__)
#    define WIN_W 1920
#    define WIN_H 1200
#else

#    define WIN_W 1920
#    define WIN_H 1080
#endif
#define CAM_PITCH_LIMIT 1.55334f
#define CAM_SPEED 5.0f
#define CAM_SENSITIVITY 0.0025f
#define FOV_Y (3.14159265f / 2.0f)

typedef struct AppState
{
    bool mouse_initialized;
    double mouse_x;
    double mouse_y;
    float cam_pitch;
    float cam_yaw;
    VEC3(float) cam_pos;
} AppState;

static void cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
    AppState* state = (AppState*)glfwGetWindowUserPointer(window);

    if (!state->mouse_initialized)
    {
        // first move not jank
        state->mouse_x = xpos;
        state->mouse_y = ypos;
        state->mouse_initialized = true;
        return;
    }

    double dx = xpos - state->mouse_x;
    double dy = ypos - state->mouse_y;

    state->cam_yaw += (float)dx * CAM_SENSITIVITY;
    state->cam_pitch += (float)dy * CAM_SENSITIVITY;

    if (state->cam_pitch > CAM_PITCH_LIMIT)
        state->cam_pitch = CAM_PITCH_LIMIT;
    if (state->cam_pitch < -CAM_PITCH_LIMIT)
        state->cam_pitch = -CAM_PITCH_LIMIT;

    state->mouse_x = xpos;
    state->mouse_y = ypos;
}

static void key_callback(GLFWwindow* window, int key, [[maybe_unused]] int scancode, int action,
                         [[maybe_unused]] int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void update_camera(GLFWwindow* window, AppState* state, float dt)
{
    float cyaw = cosf(-state->cam_yaw);
    float syaw = sinf(-state->cam_yaw);
    float cpitch = cosf(state->cam_pitch);
    float spitch = sinf(state->cam_pitch);

    float step = CAM_SPEED * dt;

    VEC3(float)
    fwd = { .x = -syaw * cpitch * step, .y = -spitch * step, .z = cyaw * cpitch * step };
    VEC3(float) right = { .x = cyaw * step, .y = 0, .z = syaw * step };
    VEC3(float) up = { .x = 0, .y = step, .z = 0 };

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        VEC3_ADD_INPLACE(&state->cam_pos, fwd);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        VEC3_SUB_INPLACE(&state->cam_pos, fwd);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        VEC3_ADD_INPLACE(&state->cam_pos, right);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        VEC3_SUB_INPLACE(&state->cam_pos, right);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        VEC3_ADD_INPLACE(&state->cam_pos, up);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        VEC3_SUB_INPLACE(&state->cam_pos, up);
}

int main(void)
{
    srand(time(NULL));

    mvp = calloc(1, sizeof(mvp_model));
    mat4_identity(mvp->model);
    mat4_identity(mvp->view);
    mat4_perspective(mvp->proj, FOV_Y, (float)WIN_W / WIN_H, 0.1f, 1000.0f);

    AppState state = {
        .mouse_initialized = false,
        .cam_pitch = 0.0f,
        .cam_yaw = 0.0f,
        .cam_pos = { .x = 0, .y = 0, .z = -3 },
    };

    GLFWwindow* win = init_window();
    glfwSwapInterval(0);

    glfwSetWindowUserPointer(win, &state);
    glfwSetKeyCallback(win, key_callback);
    glfwSetCursorPosCallback(win, cursor_callback);

    GLuint vs = compile_shader(GL_VERTEX_SHADER, "src/shaders/shader.vert");
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, "src/shaders/shader.frag");
    GLuint render_prog = create_program(vs, fs);

    // clang-format off
    float vertices[] = {
        // front
        -0.5f, -0.5f,  0.5f,   0.5f, -0.5f,  0.5f,   0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,   0.5f,  0.5f,  0.5f,  -0.5f,  0.5f,  0.5f,
        // back
         0.5f, -0.5f, -0.5f,  -0.5f, -0.5f, -0.5f,  -0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,  -0.5f,  0.5f, -0.5f,   0.5f,  0.5f, -0.5f,
        // left
        -0.5f, -0.5f, -0.5f,  -0.5f, -0.5f,  0.5f,  -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,  -0.5f,  0.5f,  0.5f,  -0.5f,  0.5f, -0.5f,
        // right
         0.5f, -0.5f,  0.5f,   0.5f, -0.5f, -0.5f,   0.5f,  0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,   0.5f,  0.5f, -0.5f,   0.5f,  0.5f,  0.5f,
        // top
        -0.5f,  0.5f,  0.5f,   0.5f,  0.5f,  0.5f,   0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f,  0.5f,   0.5f,  0.5f, -0.5f,  -0.5f,  0.5f, -0.5f,
        // bottom
        -0.5f, -0.5f, -0.5f,   0.5f, -0.5f, -0.5f,   0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,   0.5f, -0.5f,  0.5f,  -0.5f, -0.5f,  0.5f,
    };
    // clang-format on
    GLuint cubeVAO;
    GLuint cubeVBO;

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(LOCATION_POS);
    glVertexAttribPointer(LOCATION_POS, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);

    GLuint fbo, fbo_tex, fbo_depth;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGenTextures(1, &fbo_tex);
    glBindTexture(GL_TEXTURE_2D, fbo_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, WIN_W, WIN_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_tex, 0);
    glGenRenderbuffers(1, &fbo_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, fbo_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, WIN_W, WIN_H);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo_depth);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glEnable(GL_DEPTH_TEST);

    GLuint mvp_ubo;
    bind_uniform_buffer(&mvp_ubo, BINDING_MVP, mvp, sizeof(mvp_model));

    double last_t = glfwGetTime();

    while (!glfwWindowShouldClose(win))
    {
        double t0 = glfwGetTime();
        float dt = (float)(t0 - last_t);
        last_t = t0;

        update_camera(win, &state, dt);

        mat4_view_from_camera(mvp->view, state.cam_pos.x, state.cam_pos.y, state.cam_pos.z,
                              state.cam_pitch, state.cam_yaw);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, WIN_W, WIN_H);
        glClearColor(0.4f, 0.5f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(render_prog);
        glBindBuffer(GL_UNIFORM_BUFFER, mvp_ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mvp_model), mvp);

        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, WIN_W, WIN_H, 0, 0, WIN_W, WIN_H, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glfwTerminate();
    free(mvp);
}
