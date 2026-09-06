#version 330

layout(std140) uniform Settings
{
    layout(row_major) mat4 wMatrix;
    layout(row_major) mat4 vpMatrix;
    vec4 lightDir;
    vec4 diffuse;
};

in vec4 v_NORMAL;
layout(location = 0) out vec4 SV_Target;

void main()
{
    float _30 = dot(normalize(v_NORMAL.xyz), -lightDir.xyz);
    SV_Target = vec4(diffuse.xyz * (isnan(_30) ? 0.20000000298023223876953125 : (isnan(0.20000000298023223876953125) ? _30 : max(0.20000000298023223876953125, _30))), 1.0);
}

