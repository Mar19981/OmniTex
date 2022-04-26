#version 450 core




layout(location = 0) in vec4 inPosition; 
layout(location = 1) in vec2 inTexCoord; 
layout(location = 2) in vec3 inNormal; 
layout(location = 3) in vec3 inTangent; 
layout(location = 4) in vec3 inBitangent; 

layout(location = 0) out vec3 outPosition; 
layout(location = 1) out vec2 outTexCoord; 
layout(location = 2) out vec3 outNormal; 
layout(location = 3) out vec3 outTangent; 
layout(location = 4) out vec3 outBitangent; 


layout(std140, binding = 6) uniform buf {
    vec3 cameraPos; 
    vec3 lightPos; 
    vec3 ambientIntensity; 
    vec3 lightColor; 
    vec3 specularIntensity; 
    vec3 attenuation; 
    float u; 
    float v; 
    float shininess; 

} ubo;

void main()
{
    vec2 uv = vec2(ubo.u, ubo.v);
    outTexCoord = inTexCoord * uv; 
    
    outNormal = inNormal; 
    outPosition = vec3(inPosition);
    outTangent = inTangent;
    outBitangent = inBitangent;
} 
