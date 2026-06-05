#pragma once

#include <glad/glad.h>
// glad before
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include "voxel/face.h"
#include "utils/container.h"

VECTOR_DECLARE(Face);

GLuint describe_faces(GLuint cube_vao);
