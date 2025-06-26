#version 450

layout(binding = 0) uniform sampler2D sceneTex;
layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

const float threshold = 1.0;

void main() {
    vec3 sceneColor = texture(sceneTex, uv).rgb;
    outColor = vec4(sceneColor, 1.0);
/*
    vec3 color = texture(sceneTex, uv).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    outColor = brightness > threshold ? vec4(color, 1.0) : vec4(0.0);*/
}