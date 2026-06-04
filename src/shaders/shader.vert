#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in int face;
layout(location = 1) in vec3 pos;

const vec3 faceOffsets[6][4] = {
    {vec3(1,0,0), vec3(1,1,0), vec3(1,1,1), vec3(1,0,1)}, // +X
    {vec3(0,0,1), vec3(0,1,1), vec3(0,1,0), vec3(0,0,0)}, // -X
    {vec3(0,1,1), vec3(1,1,1), vec3(1,1,0), vec3(0,1,0)}, // +Y
    {vec3(0,0,0), vec3(1,0,0), vec3(1,0,1), vec3(0,0,1)}, // -Y
    {vec3(1,0,1), vec3(1,1,1), vec3(0,1,1), vec3(0,0,1)}, // +Z
    {vec3(0,0,0), vec3(0,1,0), vec3(1,1,0), vec3(1,0,0)}, // -Z
};

const vec2 faceUVs[6][4] = {
    {vec2(0,1), vec2(1,1), vec2(1,0), vec2(0,0)}, // +X
    {vec2(1,0), vec2(0,0), vec2(0,1), vec2(1,1)}, // -X
    {vec2(1,0), vec2(0,0), vec2(0,1), vec2(1,1)}, // +Y
    {vec2(0,1), vec2(1,1), vec2(1,0), vec2(0,0)}, // -Y
    {vec2(1,0), vec2(0,0), vec2(0,1), vec2(1,1)}, // +Z
    {vec2(0,1), vec2(1,1), vec2(1,0), vec2(0,0)}, // -Z
};

const int quadIndices[6] = {0, 1, 2, 2, 3, 0};

void main() {
    int v = quadIndices[gl_VertexID % 6];

    vec3 worldPos = pos + faceOffsets[face][v];
    gl_Position  = ubo.proj * ubo.view * ubo.model * vec4(worldPos, 1.0);
    //fragTexCoord = vec3(faceUVs[inFaceId][v], float(inTextureId));
}
