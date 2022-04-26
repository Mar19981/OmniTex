#version 450 core




layout(location = 0) in vec3 inPosition[]; 
layout(location = 1) in vec2 inTexCoord[]; 
layout(location = 2) in vec3 inNormal[]; 
layout(location = 3) in vec3 inTangent[]; 
layout(location = 4) in vec3 inBitangent[]; 
layout(triangles) in; 



layout(location = 0) out vec2 outTexCoords; 
layout(location = 1) out mat3 outTBN; 
layout(location = 4) out vec3 fragPos; 

layout(std140, binding = 0) uniform buf {
    mat4 model; 
    mat4 view; 
    mat4 projection; 
    float displacementFactor; 
} ubo;

layout(binding = 3) uniform sampler2D displacement; 


vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)
{
    return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
} 

void main() {
    
    outTexCoords = interpolate2D(inTexCoord[0], inTexCoord[1], inTexCoord[2]);
    
    vec3 normal = interpolate3D(inNormal[0], inNormal[1], inNormal[2]), 
        N = normalize(vec3(ubo.model * vec4(normal, 1.0))),
        T = normalize(vec3(ubo.model * vec4(interpolate3D(inTangent[0], inTangent[1], inTangent[2]), 1.0))),
        B = normalize(vec3(ubo.model * vec4(interpolate3D(inBitangent[0], inBitangent[1], inBitangent[2]), 1.0)));
    
    vec4 newPos = vec4(interpolate3D(inPosition[0], inPosition[1], inPosition[2]), 1.0);
    
    float disp = texture(displacement, outTexCoords).r;
    newPos.xyz += normalize(normal) * disp * ubo.displacementFactor;
    
    fragPos = vec3(ubo.model * newPos);
    
    outTBN = mat3(T, B, N);
    
    gl_Position = ubo.projection * ubo.view * ubo.model * newPos;
}