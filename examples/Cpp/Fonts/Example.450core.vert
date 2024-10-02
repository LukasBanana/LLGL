// GLSL model vertex shader

#version 450 core

layout(push_constant) uniform Scene
{
    mat4 projection;
    vec2 glyphAtlasInvSize;
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
    gl_Position = projection * vec4(x, y, 0, 1);
    vTexCoord = glyphAtlasInvSize * vec2(u, v);
    vColor    = color;
}
