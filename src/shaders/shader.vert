#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in int face;
layout(location = 1) in ivec3 pos;

const ivec3 faceOffsets[6][4] = {
    {ivec3(1,0,0), ivec3(1,1,0), ivec3(1,1,1), ivec3(1,0,1)}, // +X
    {ivec3(0,0,1), ivec3(0,1,1), ivec3(0,1,0), ivec3(0,0,0)}, // -X
    {ivec3(0,1,1), ivec3(1,1,1), ivec3(1,1,0), ivec3(0,1,0)}, // +Y
    {ivec3(0,0,0), ivec3(1,0,0), ivec3(1,0,1), ivec3(0,0,1)}, // -Y
    {ivec3(1,0,1), ivec3(1,1,1), ivec3(0,1,1), ivec3(0,0,1)}, // +Z
    {ivec3(0,0,0), ivec3(0,1,0), ivec3(1,1,0), ivec3(1,0,0)}, // -Z
};

const ivec2 faceUVs[6][4] = {
    {ivec2(0,1), ivec2(1,1), ivec2(1,0), ivec2(0,0)}, // +X
    {ivec2(1,0), ivec2(0,0), ivec2(0,1), ivec2(1,1)}, // -X
    {ivec2(1,0), ivec2(0,0), ivec2(0,1), ivec2(1,1)}, // +Y
    {ivec2(0,1), ivec2(1,1), ivec2(1,0), ivec2(0,0)}, // -Y
    {ivec2(1,0), ivec2(0,0), ivec2(0,1), ivec2(1,1)}, // +Z
    {ivec2(0,1), ivec2(1,1), ivec2(1,0), ivec2(0,0)}, // -Z
};

const int quadIndices[6] = {0, 1, 2, 2, 3, 0};

void main() {
    int v = quadIndices[gl_VertexID % 6];

    ivec3 worldPos = pos + faceOffsets[face][v];
    gl_Position  = ubo.proj * ubo.view * ubo.model * ivec4(worldPos, 1.0);
    //fragTexCoord = ivec3(faceUVs[inFaceId][v], float(inTextureId));
}
