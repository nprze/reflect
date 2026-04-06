#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in uint inObjectIndex;
layout(location = 3) in float inPerPrimitiveFluctuate;

layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform uboData {
    mat4 vp;
    float globalTime;
} ubo;

layout(set = 1, binding = 1) readonly buffer ObjectMatrices {
    mat4 modelMatrices[];
};
void main() {
    gl_Position = ubo.vp * modelMatrices[inObjectIndex] * vec4(inPosition, 1.0);
    fragColor = inColor * sin(ubo.globalTime + inPerPrimitiveFluctuate);
}
