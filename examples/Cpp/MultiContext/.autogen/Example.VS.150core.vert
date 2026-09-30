#version 150

in vec2 POSITION;
in vec3 COLOR;
out vec3 v_COLOR;

void main()
{
    gl_Position = vec4(POSITION, 0.0, 1.0);
    v_COLOR = COLOR;
}

