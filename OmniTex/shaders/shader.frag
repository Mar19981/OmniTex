#version 450 core

layout(location = 0) in vec2 inTexCoords; 
layout(location = 1) in mat3 inTBN; 
layout(location = 4) in vec3 fragPos; 

layout(location = 0) out vec4 fragColor; 

layout(binding = 1) uniform sampler2D diffuse; 
layout(binding = 2) uniform sampler2D specular; 
layout(binding = 4) uniform sampler2D normal; 
layout(binding = 5) uniform sampler2D ao; 


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
    vec3 normalMap = texture(normal, inTexCoords).rgb;

    vec3 vertexNormal = normalize(normalMap * 2.0 - 1.0);
    vertexNormal = normalize(inTBN * vertexNormal);
    vec3 lightDirection = normalize(ubo.lightPos - fragPos);
    float diffuseImpact = max(dot(vertexNormal, lightDirection), 0.0);

    vec3 viewDirection = normalize(ubo.cameraPos - fragPos),
        reflectionDirection = reflect(-lightDirection, vertexNormal);
    float specularImpact = pow(max(dot(viewDirection, reflectionDirection), 0.0), ubo.shininess);


    float dist = length(ubo.lightPos - fragPos),
    attenuation = 1.0 / (ubo.attenuation.x + ubo.attenuation.y * dist + ubo.attenuation.z * dist * dist);

    vec4 aoColor = texture(ao, inTexCoords), 
        diffuseColor = texture(diffuse, inTexCoords) * aoColor,
        specularMap = texture(specular, inTexCoords);
    vec3 ambient = ubo.ambientIntensity * vec3(diffuseColor),
        diffuse = ubo.lightColor * diffuseImpact * vec3(diffuseColor),
        specularValue = ubo.specularIntensity * specularImpact * vec3(specularMap);
    ambient *= attenuation;
    specularValue *= attenuation;
    diffuse *= attenuation;
    fragColor = vec4(ambient + diffuse + specularValue, 1.0);
}
