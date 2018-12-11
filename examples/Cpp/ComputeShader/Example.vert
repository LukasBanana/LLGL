// GLSL vertex shader

#version 450 core

layout(location = 0) in vec2 coord;
layout(location = 1) in vec4 color;
layout(location = 2) in mat2 rotation;
layout(location = 4) in vec2 position;

layout(location = 0) out vec4 vColor;

out gl_PerVertex
{
    vec4 gl_Position;
};

// Vertex shader main function
void main()
{
    gl_Position = vec4(rotation * coord + position, 0, 1);
    vColor      = color;
}


