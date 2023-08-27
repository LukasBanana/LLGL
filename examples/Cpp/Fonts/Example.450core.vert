// GLSL model vertex shader

#version 450 core

layout(std140, binding = 0) uniform Settings
{
    mat4 wvpMatrix;
    vec2 glyphTextureInvSize;
};

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(location = 0) in ivec2 position;
layout(location = 1) in ivec2 texCoord;
layout(location = 2) in vec4 color;

layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec4 vColor;

void main()
{
    // Decompress vertex attributes
    float x = float(position.x);
    float y = float(position.y);
    float u = float(texCoord.x);
    float v = float(texCoord.y);

    // Write vertex output attributes
    gl_Position = wvpMatrix * vec4(x, y, 0, 1);
    vTexCoord = glyphTextureInvSize * vec2(u, v);
    vColor    = color;
}
