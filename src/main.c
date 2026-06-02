#include <float.h>
#include <glad/glad.h>
// glad
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "headers.h"
#include "shader_simulation.h"
#include "utils/utils.h"

typedef struct AppState
{
    bool step;
    bool step_mode;
    bool draw_densities;
    bool draw_chunks;
    bool draw_particles;
    bool reset;
    bool mouse_initialized;
    double mouse_x;
    double mouse_y;
    shader_simulation* s;
    int var_index;
} AppState;

#define CAM_PITCH_LIMIT 1.55334f

static void cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
    AppState* state = (AppState*)glfwGetWindowUserPointer(window);

    if (!state->mouse_initialized)
    {
        // first move is not jank
        state->mouse_x = xpos;
        state->mouse_y = ypos;
        state->mouse_initialized = true;
        return;
    }

    double dx = xpos - state->mouse_x;
    double dy = ypos - state->mouse_y;

    state->s->cam_yaw += (float)dx * 0.0025;
    state->s->cam_pitch += (float)dy * 0.0025;

    if (state->s->cam_pitch > CAM_PITCH_LIMIT)
        state->s->cam_pitch = CAM_PITCH_LIMIT;
    if (state->s->cam_pitch < -CAM_PITCH_LIMIT)
        state->s->cam_pitch = -CAM_PITCH_LIMIT;

    state->mouse_x = xpos;
    state->mouse_y = ypos;
}

static void key_callback(GLFWwindow* window, int key, [[maybe_unused]] int scancode, int action,
                         [[maybe_unused]] int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    AppState* state = ((AppState*)glfwGetWindowUserPointer(window));

    if (action != GLFW_PRESS)
        return;

    if (key == GLFW_KEY_N)
        state->step = true;

    if (key == GLFW_KEY_P || key == GLFW_KEY_ENTER)
        state->step_mode = !state->step_mode;

    if (key == GLFW_KEY_R)
        state->reset = true;

    if (key == GLFW_KEY_G)
        state->draw_chunks = !state->draw_chunks;

    if (key == GLFW_KEY_M)
        state->draw_particles = !state->draw_particles;

    if (key == GLFW_KEY_RIGHT && state->var_index < 7)
        state->var_index += 1;
    if (key == GLFW_KEY_LEFT && state->var_index > 0)
        state->var_index -= 1;
}

static void update_camera(GLFWwindow* window, AppState* state, float dt)
{
    float cos_pitch = cosf(state->s->cam_pitch);
    float sin_pitch = sinf(state->s->cam_pitch);
    float cos_yaw = cosf(-state->s->cam_yaw);
    float sin_yaw = sinf(-state->s->cam_yaw);

    float step = 150 * dt;

    VEC3(float) fwd = {
        .x = -sin_yaw * cos_pitch * step,
        .y = -sin_pitch * step,
        .z = cos_yaw * cos_pitch * step,
    };

    VEC3(float) right = {
        .x = cos_yaw * step,
        .y = 0,
        .z = sin_yaw * step,
    };

    VEC3(float) up = {
        .x = 0,
        .y = step,
        .z = 0,
    };

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        VEC3_ADD_INPLACE(&state->s->cam_pos, fwd);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        VEC3_SUB_INPLACE(&state->s->cam_pos, fwd);

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        VEC3_ADD_INPLACE(&state->s->cam_pos, right);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        VEC3_SUB_INPLACE(&state->s->cam_pos, right);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        VEC3_ADD_INPLACE(&state->s->cam_pos, up);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        VEC3_SUB_INPLACE(&state->s->cam_pos, up);
}

#define PRINT_SSBO(ssbo, type, nb_elts, print_func)                                                \
    do                                                                                             \
    {                                                                                              \
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);                                              \
        type* arr = (type*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);                    \
        printf("%s\n", #ssbo);                                                                     \
        for (int i = 0; i < nb_elts; i++)                                                          \
        {                                                                                          \
            print_func(arr, i);                                                                    \
        }                                                                                          \
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);                                                   \
    } while (0)

void print_float(float* arr, int index)
{
    printf("%i : %f\n", index, arr[index]);
}

void print_vec3(VEC3(float)* arr, int index)
{
    printf("arr[%i] = ", index);
    printf("(%f, %f, %f)\n", arr[index].x, arr[index].y, arr[index].z);
}

int main()
{
    srand(time(NULL));

    s = calloc(1, sizeof(shader_simulation));

    AppState state = {
        .step = false,
        .step_mode = true,
        .mouse_initialized = false,
        .s = s,
        .var_index = 0,
    };

    s->cam_pos = (VEC3(float)){
        .x = 10,
        .y = 10,
        .z = 10,
    };

    GLFWwindow* win = init_window();
    glfwSwapInterval(0);

    uint32_t win_w = 1920;
    uint32_t win_h = 1080;

    glfwSetWindowUserPointer(win, &state);
    glfwSetKeyCallback(win, key_callback);
    glfwSetCursorPosCallback(win, cursor_callback);

    GLuint vs = compile_shader(GL_VERTEX_SHADER, "shaders/shader.vert");
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, "shaders/shader.frag");
    GLuint render_prog = create_program(vs, fs);

    glEnable(GL_PROGRAM_POINT_SIZE);

    /*GLuint particles_buffer;
    glGenBuffers(1, &particles_buffer);

    glBindBuffer(GL_ARRAY_BUFFER, particles_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(shader_particle) * NB_PARTICLES, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_PARTICLES, particles_buffer);*/

    /*GLuint particles_ssbo =
        opengl_add_array(NULL, sizeof(shader_particle) * NB_PARTICLES, BINDING_PARTICLES);
    GLuint pred_pos_ssbo = opengl_add_array(NULL, 16 * NB_PARTICLES, BINDING_PREDICTED_POSITIONS);
    GLuint densities_ssbo = opengl_add_array(NULL, sizeof(float) * NB_PARTICLES, BINDING_DENSITIES);
    GLuint start_chunks_ssbo =
        opengl_add_array(NULL, sizeof(uint32_t) * c->nb_chunk_x * c->nb_chunk_y * c->nb_chunk_z,
                         BINDING_START_CHUNKS);
    GLuint pairs_ssbo =
        opengl_add_array(pairs, sizeof(chunk_particle_idx_pair) * n2, BINDING_PAIRS);
    GLuint existence_field_ssbo =
        opengl_add_array(NULL, sizeof(GLuint) * c->nb_chunk_x * c->nb_chunk_y * c->nb_chunk_z,
                         BINDING_EXISTENCE_FIELD);*/

    float vertices[] = { -1.0f, -1.0f, 3.0f, -1.0f, -1.0f, 3.0f };

    unsigned int quadVAO, quadVBO;

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(LOCATION_POS);
    glVertexAttribPointer(LOCATION_POS, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);

    GLuint fbo, fbo_tex;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGenTextures(1, &fbo_tex);
    glBindTexture(GL_TEXTURE_2D, fbo_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, win_w, win_h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_tex, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    bind_uniform_buffer(&s->simulation_ubo, BINDING_SIMULATION, s, sizeof(shader_simulation));

    double last_t = glfwGetTime();

    while (!glfwWindowShouldClose(win))
    {
        double t0 = glfwGetTime();

        s->dt = (float)(t0 - last_t);
        last_t = t0;

        START_TIME(camera);
        update_camera(win, &state, s->dt);
        END_TIME(camera);

        double t1 = glfwGetTime();

        START_TIME(render);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, win_w, win_h);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(render_prog);
        glBindBuffer(GL_UNIFORM_BUFFER, s->simulation_ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(shader_simulation), s);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, win_w, win_h, 0, 0, win_w, win_h, GL_COLOR_BUFFER_BIT,
                          GL_LINEAR);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        END_TIME(render);

        START_TIME(swap);
        glfwSwapBuffers(win);
        glfwPollEvents();
        END_TIME(swap);

        double t2 = glfwGetTime();

        printf("%f %f\n", s->cam_yaw, s->cam_pitch);
        print_vec3(&s->cam_pos, 0);
        //PRINT_TIMINGS(1.0 / (t2 - t0));
        //fflush(stdout);
    }

    glfwTerminate();
}
