#pragma once

#include <glad/glad.h>
// glad before
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include "utils/vec3.h"
#include "voxel/world.h"

#define CAM_PITCH_LIMIT 1.55334f
#define CAM_SPEED 40.0f
#define CAM_SENSITIVITY 0.0025f

typedef struct AppState
{
    bool mouse_initialized;
    double mouse_x;
    double mouse_y;
    float cam_pitch;
    float cam_yaw;
    World* w;
} AppState;

void key_callback(GLFWwindow* window, int key, [[maybe_unused]] int scancode, int action,
                  [[maybe_unused]] int mods);
void cursor_callback(GLFWwindow* window, double xpos, double ypos);
void update_camera(GLFWwindow* window, AppState* state, float dt);
