#version 450

layout(binding = 0) uniform sampler2D sceneTex;
layout(binding = 1) uniform sampler2D bloomTex;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

void main() {
    vec3 sceneColor = texture(sceneTex, uv).rgb;
    vec3 bloomColor = texture(bloomTex, uv).rgb;

    vec3 finalColor = sceneColor + bloomColor * 0.4;
    outColor = vec4(finalColor, 1.0);
}
