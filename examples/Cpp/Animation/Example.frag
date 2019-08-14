// GLSL fragment shader

#version 140

layout(std140) uniform Settings
{
    mat4 wMatrix;
    mat4 vpMatrix;
    vec4 lightDir;
    vec4 diffuse;
};

in vec4 vNormal;
in vec2 vTexCoord;

out vec4 fColor;

uniform sampler2D colorMap;

void main()
{
    // Compute lighting
    vec3 normal = normalize(vNormal.xyz);
    float NdotL = max(0.2, dot(normal, -lightDir.xyz));

    // Sample texture
    vec4 color = texture(colorMap, vTexCoord);

    // Set final output color
    fColor = mix(vec4(1), color, diffuse.a) * vec4(diffuse.rgb * NdotL, 1.0);
}



