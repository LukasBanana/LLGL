// GLSL stencil fragment shader

#version 450 core

layout(std140, binding = 1) uniform Settings
{
    mat4 wMatrix;
    mat4 vpMatrix;
    vec4 lightDir;
    vec4 diffuse;
};

layout(location = 0) in vec4 vNormal;

layout(location = 0) out vec4 fColor;

void main()
{
    // Compute lighting
    vec3 normal = normalize(vNormal.xyz);
    float NdotL = max(0.2, dot(normal, -lightDir.xyz));
    
    // Set final output color
    fColor = vec4(diffuse.rgb * NdotL, 1.0);
}



