#version 330

layout(std140) uniform SceneView
{
    layout(row_major) mat4 wMatrix;
    layout(row_major) mat4 vpMatrix;
    vec3 lightDir;
    float reflectionFading;
    vec4 diffuse;
};

in vec3 v_NORMAL;
layout(location = 0) out vec4 SV_Target;

void main()
{
    vec3 _33 = -lightDir;
    float _34 = dot(normalize(vec3(0.0, 1.0, 0.0)), _33);
    float _44 = dot(normalize(v_NORMAL), _33);
    SV_Target = mix(vec4(diffuse.xyz * (isnan(_44) ? 0.20000000298023223876953125 : (isnan(0.20000000298023223876953125) ? _44 : max(0.20000000298023223876953125, _44))), diffuse.w), vec4(vec3(1.0) * (isnan(_34) ? 0.20000000298023223876953125 : (isnan(0.20000000298023223876953125) ? _34 : max(0.20000000298023223876953125, _34))), 1.0), vec4(reflectionFading));
}

