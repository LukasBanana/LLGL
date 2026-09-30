#version 130

in vec3 v_COLOR;
out vec4 SV_Target;

void main()
{
    SV_Target = vec4(v_COLOR, 1.0);
}

