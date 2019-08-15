// GLSL fragment shader

#version 140

layout(std140) uniform Settings
{
    mat4    wMatrix;
    mat4    vpMatrix;
    vec3    lightDir;
    float   shininess;
    vec4    viewPos;
    vec4    albedo;
};

in vec4 vWorldPos;
in vec4 vNormal;
in vec2 vTexCoord;

out vec4 fColor;

uniform sampler2D colorMap;

void main()
{
    // Diffuse lighting
    vec3    lightVec    = -lightDir.xyz;
    vec3    normal      = normalize(vNormal.xyz);
    float   NdotL       = mix(0.2, 1.0, max(0.0, dot(normal, lightVec)));
    vec3    diffuse     = albedo.rgb * NdotL;

    // Specular lighting
    vec3    viewDir     = normalize(viewPos.xyz - vWorldPos.xyz);
    vec3    halfVec     = normalize(viewDir + lightVec);
    float   NdotH       = dot(normal, halfVec);
    vec3    specular    = vec3(pow(max(0.0, NdotH), shininess));

    // Sample texture
    vec4 color = texture(colorMap, vTexCoord);

    // Set final output color
    fColor = mix(vec4(1), color, albedo.a) * vec4(diffuse + specular, 1.0);
}



