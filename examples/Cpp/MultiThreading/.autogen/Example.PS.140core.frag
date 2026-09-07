#version 140

layout(std140) uniform Scene
{
    layout(row_major) mat4 wvpMatrix;
    layout(row_major) mat4 wMatrix;
    vec3 lightVec;
};

in vec3 v_NORMAL;
in vec4 v_COLOR;
out vec4 SV_Target;

void main()
{
    vec3 _32 = v_COLOR.xyz * mix(0.20000000298023223876953125, 1.0, dot(lightVec, normalize(v_NORMAL)));
    SV_Target = vec4(_32.x, _32.y, _32.z, v_COLOR.w);
}

