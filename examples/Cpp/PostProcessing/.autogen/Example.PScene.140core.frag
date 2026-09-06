#version 140

layout(std140) uniform SceneSettings
{
    layout(row_major) mat4 wvpMatrix;
    layout(row_major) mat4 wMatrix;
    vec4 diffuse;
    vec4 glossiness;
    vec3 lightDir;
    float intensity;
};

in vec3 v_NORMAL;
out vec4 SV_Target0;
out vec4 SV_Target1;

void main()
{
    float _30 = dot(lightDir, normalize(v_NORMAL));
    float _31 = isnan(_30) ? 0.4000000059604644775390625 : (isnan(0.4000000059604644775390625) ? _30 : max(0.4000000059604644775390625, _30));
    SV_Target0 = diffuse * vec4(_31, _31, _31, 1.0);
    SV_Target1 = glossiness;
}

