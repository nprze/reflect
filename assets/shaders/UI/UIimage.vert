#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in int inTexIndex;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out flat int texIndex;

layout(set = 0, binding = 0) uniform cameraUbo {
    mat4 vp;
} ubo;

void main() {
    gl_Position = ubo.vp * vec4(inPosition, 0.0, 1.0);
    fragTexCoord = texCoord;
    texIndex = inTexIndex;
}
