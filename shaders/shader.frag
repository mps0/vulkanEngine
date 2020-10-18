#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

void main() {

    vec3 lightPos = vec3(0.f, 5.f, -10.0f);
    vec3 lightVec = fragPos - lightPos;
    vec3 lightDir = normalize(lightVec);
    float lightDistSq = dot(lightVec, lightVec);
   
    float lightConstant = 1.f;
    float lightIntensity = 1.f / (lightConstant + lightDistSq);

    float cosTerm = max(-dot(lightDir, fragNormal), 0.f);

    outColor = vec4(10.f * lightIntensity * cosTerm * fragColor, 1.f);
    
    
}
