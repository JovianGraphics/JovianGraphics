#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform Constants {
    mat4 viewMtx;
    mat4 projMtx;
    uint numLights;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in mat4 modelMtx;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec4 pos;

void main() {
    pos = viewMtx * (modelMtx * vec4(inPosition, 1.0));
    fragColor = inColor;
    normal = normalize(mat3(viewMtx) * mat3(modelMtx) * inNormal);

    gl_Position = projMtx * pos;
}