#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragPos;

layout (std140, set = 0, binding = 0) uniform bufM {
    mat4 model;
} uboM;

layout (std140, set = 1, binding = 0) uniform bufVP {
    mat4 view;
    mat4 projection;
} uboVP;

void main() {

    mat4 MVP = uboVP.projection * uboVP.view * uboM.model;
    gl_Position = MVP * vec4(pos, 1.f);
    fragColor = color;
    fragNormal = normal;
    fragPos = vec3(uboM.model * vec4(pos, 1.f));
}
