#version 120

attribute vec2 POSITION;
attribute vec3 COLOR;
varying vec3 v_COLOR;

void main()
{
    gl_Position = vec4(POSITION, 0.0, 1.0);
    v_COLOR = COLOR;
}

