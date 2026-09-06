#version 420

layout(binding = 1, std140) uniform Settings
{
    layout(row_major) mat4 cMatrix;
    layout(row_major) mat4 vpMatrix;
    layout(row_major) mat4 wMatrix;
    vec2 aspectRatio;
    float mipCount;
    float _pad0;
    vec4 lightDir;
    uint skyboxLayer;
    uint materialLayer;
    uvec2 _pad1;
};

layout(binding = 3) uniform samplerCubeArray s_skyBoxsmpl;

layout(location = 0) in vec4 v_VIEWRAY;
layout(location = 0) out vec4 SV_Target;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    SV_Target = texture(s_skyBoxsmpl, vec4(normalize(vec4(v_VIEWRAY.xy, gl_FrontFacing ? (-1.0) : 1.0, 0.0) * spvWorkaroundRowMajor(cMatrix)).xyz, float(skyboxLayer)));
}

