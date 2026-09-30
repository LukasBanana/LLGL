#version 140

layout(std140) uniform Settings
{
    layout(row_major) mat4 wvpMatrix;
    layout(row_major) mat4 wMatrix;
    vec4 color;
    vec3 lightDir;
};

in vec3 v_NORMAL;
out vec4 SV_Target;

void main()
{
    float _28 = dot(lightDir, normalize(v_NORMAL));
    float _29 = isnan(_28) ? 0.20000000298023223876953125 : (isnan(0.20000000298023223876953125) ? _28 : max(0.20000000298023223876953125, _28));
    SV_Target = color * vec4(_29, _29, _29, 1.0);
}

