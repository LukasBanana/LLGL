// GLSL model vertex shader

#version 140

#ifdef GL_ES
precision mediump float;
#endif

uniform mat4 projection;
uniform vec2 glyphAtlasInvSize;

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
    gl_Position = projection * vec4(x, y, 0, 1);
    vTexCoord = glyphAtlasInvSize * vec2(u, v);
    vColor    = color;
}
