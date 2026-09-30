#version 300 es

layout(location = 0) in vec2 POSITION;
layout(location = 1) in vec3 COLOR;
out vec3 v_COLOR;

void main()
{
    gl_Position = vec4(POSITION, 0.0, 1.0);
    v_COLOR = COLOR;
}

