#version 150

in vec3 g_COLOR;
out vec4 SV_Target;

void main()
{
    SV_Target = vec4(g_COLOR, 1.0);
}

