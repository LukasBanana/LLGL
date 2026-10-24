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
    float _30 = dot(normalize(v_NORMAL), -lightDir);
    SV_Target = vec4(diffuse.xyz * (isnan(_30) ? 0.20000000298023223876953125 : (isnan(0.20000000298023223876953125) ? _30 : max(0.20000000298023223876953125, _30))), diffuse.w);
}

