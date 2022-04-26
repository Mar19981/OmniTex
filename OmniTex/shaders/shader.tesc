
#version 450 core




layout(location = 0) in vec3 inPosition[]; 
layout(location = 1) in vec2 inTexCoord[]; 
layout(location = 2) in vec3 inNormal[]; 
layout(location = 3) in vec3 inTangent[]; 
layout(location = 4) in vec3 inBitangent[]; 


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


float tessellationLevel(float dist1, float dist2) {
    float avg = (dist1 + dist2) * 0.5;
    if (avg <= 2.0) return 20.0;
    if (avg <= 5.0) return 14.0;
    return 6.0;
}


layout(vertices = 3) out; 
layout(location = 0) out vec3 outPosition[]; 
layout(location = 1) out vec2 outTexCoord[]; 
layout(location = 2) out vec3 outNormal[]; 
layout(location = 3) out vec3 outTangent[]; 
layout(location = 4) out vec3 outBitangent[]; 

void main() {
    if (gl_InvocationID == 0) {
        
        float cameraDistance1 = distance(ubo.cameraPos, inPosition[0]),
            cameraDistance2 = distance(ubo.cameraPos, inPosition[1]),
            cameraDistance3 = distance(ubo.cameraPos, inPosition[2]);
        
        gl_TessLevelOuter[0] = tessellationLevel(cameraDistance2, cameraDistance3);
        gl_TessLevelOuter[1] = tessellationLevel(cameraDistance3, cameraDistance1);
        gl_TessLevelOuter[2] = tessellationLevel(cameraDistance1, cameraDistance2);
        gl_TessLevelInner[0] = gl_TessLevelOuter[2];
    }
    
    outPosition[gl_InvocationID] = inPosition[gl_InvocationID];
    outTexCoord[gl_InvocationID] = inTexCoord[gl_InvocationID];
    outNormal[gl_InvocationID] = inNormal[gl_InvocationID];
    outTangent[gl_InvocationID] = inTangent[gl_InvocationID];
    outBitangent[gl_InvocationID] = inBitangent[gl_InvocationID];
}