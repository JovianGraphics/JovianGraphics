#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 pos;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 color = fragColor;

    vec3 lightPos = vec3(0.0, 1.0, 0.0);

    vec3 L = normalize(lightPos - pos.xyz);

    vec3 N = normal;
    vec3 V = normalize(-pos.xyz);
    vec3 H = normalize(V + L);
    color *= max(0.0, dot(N, L)) * 0.5 + pow(max(0.0, dot(H, N)), 15.0);

    outColor = vec4(color, 1.0);
}