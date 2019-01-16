// GLSL final fragment shader

#version 140

layout(std140) uniform SceneSettings
{
    mat4    wvpMatrix;
    mat4    wMatrix;
    vec4    diffuse;
    vec4    glossiness;
    float   intensity;
};

uniform sampler2D colorMap;
uniform sampler2D glossMap;

in vec2 vTexCoord;

out vec4 fragColor;

void main()
{
    // Show final result with color and gloss map
    fragColor =
        texture(colorMap, vTexCoord) +
        texture(glossMap, vTexCoord) * intensity;
}
