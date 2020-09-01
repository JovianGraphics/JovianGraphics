#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec4 outColor;

const vec3 sunDir = normalize(vec3(0.0, 1.0, 0.0));

void main() {
    vec3 color = fragColor;

    // color *= max(0.0, dot(normal, sunDir)) * 0.9 + 0.1;

    outColor = vec4(color, 1.0);
}