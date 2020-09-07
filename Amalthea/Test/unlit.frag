#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform Constants {
    mat4 viewMtx;
    mat4 projMtx;
    uint numLights;
};

struct Light
{
    vec4 pos;
    vec4 color;
};

layout(std430, binding = 1) buffer lightBuffer
{
    Light lights[];
};

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 pos;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 color = vec3(0.0);

    for (uint i = 0; i < numLights; i++)
    {
        vec3 lightPos = lights[i].pos.rgb;
        vec3 lightColor = lights[i].color.rgb;

        vec3 L = normalize(lightPos - pos.xyz);

        vec3 N = normal;
        vec3 V = normalize(-pos.xyz);
        vec3 H = normalize(V + L);
        color += fragColor * (max(0.0, dot(N, L)) * 0.4 + pow(max(0.0, dot(H, N)), 15.0) * 0.5 + 0.1) * lightColor;
    }

    outColor = vec4(color, 1.0);
}