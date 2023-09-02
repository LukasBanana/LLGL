// GLSL model vertex shader

#version 140

#ifdef GL_ES
precision mediump float;
#endif

layout(std140) uniform Settings
{
    mat4 wvpMatrix;
    vec2 glyphTextureInvSize;
};

in ivec2 position;
in ivec2 texCoord;
in vec4 color;

out vec2 vTexCoord;
out vec4 vColor;

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
