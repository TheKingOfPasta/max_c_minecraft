#include "inputs.h"

#include <math.h>

void key_callback(GLFWwindow* window, int key, [[maybe_unused]] int scancode, int action,
                         [[maybe_unused]] int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void cursor_callback(GLFWwindow* window, double xpos, double ypos)
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

void update_camera(GLFWwindow* window, AppState* state, float dt)
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
        VEC3_ADD_INPLACE(&state->w->player->pos, fwd);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        VEC3_SUB_INPLACE(&state->w->player->pos, fwd);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        VEC3_ADD_INPLACE(&state->w->player->pos, right);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        VEC3_SUB_INPLACE(&state->w->player->pos, right);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        VEC3_ADD_INPLACE(&state->w->player->pos, up);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        VEC3_SUB_INPLACE(&state->w->player->pos, up);
}

