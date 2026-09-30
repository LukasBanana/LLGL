#version 450

layout(location = 0) in vec4 v_COLOR;
layout(location = 0) out vec4 SV_Target0;

void main()
{
    SV_Target0 = v_COLOR;
}

