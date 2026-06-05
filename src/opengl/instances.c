#include "instances.h"

GLuint describe_faces(GLuint cube_vao)
{
    GLuint instances_vbo;
    glGenBuffers(1, &instances_vbo);

    glBindVertexArray(cube_vao);
    glBindBuffer(GL_ARRAY_BUFFER, instances_vbo);

    glVertexAttribIPointer(0, 1, GL_INT,   sizeof(Face), (void*)offsetof(Face, face_id));
    glEnableVertexAttribArray(0);
    glVertexAttribDivisor(0, 1);


    glVertexAttribIPointer(1, 1, GL_INT, sizeof(Face),
                           (void*)offsetof(Face, texture_id));

    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    glVertexAttribIPointer(2, 3, GL_INT, sizeof(Face), (void*)offsetof(Face, pos));

    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    return instances_vbo;
}
