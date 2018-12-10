// GLSL vertex shader

#version 450

in vec2 coord;
in vec4 color;
in mat2 rotation;
in vec2 position;

out vec4 vColor;

// Vertex shader main function
void main()
{
    gl_Position = vec4(rotation * coord + position, 0, 1);
    vColor      = color;
}


