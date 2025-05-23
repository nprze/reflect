#version 450
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in flat int texIndex;
layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D textureSampler[10];

void main() {
    outColor = texture(textureSampler[texIndex], fragTexCoord);
}