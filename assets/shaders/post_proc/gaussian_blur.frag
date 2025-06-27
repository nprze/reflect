#version 450

layout(binding = 0) uniform sampler2D bloomTex;

layout(push_constant) uniform BlurParams {
    vec2 dir; // (1.0, 0.0) for horizontal, (0.0, 1.0) for vertical
    float res; // texture size in that direction
} params;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

void main() {
    vec3 result = vec3(0.0);
    float weights[10] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216, 
                                 0.004, 0.002, 0.001, 0.0005, 0.00025);

    result += texture(bloomTex, uv).rgb * weights[0];

    for (int i = 1; i < 10; ++i) {
        vec2 offset = params.dir * float(i) / params.res;
        vec3 sample1 = texture(bloomTex, uv + offset).rgb;
        vec3 sample2 = texture(bloomTex, uv - offset).rgb;
        result += (sample1 + sample2) * weights[i];
    }

    outColor = vec4(result, 1.0);
}
