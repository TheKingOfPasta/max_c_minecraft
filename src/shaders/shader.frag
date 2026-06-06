#version 460

layout(binding = 0) uniform sampler2DArray texSampler;

layout(location = 0) in vec3 fragTexCoord;
layout(location = 1) in float fragAO;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, fragTexCoord) * fragAO;
}

