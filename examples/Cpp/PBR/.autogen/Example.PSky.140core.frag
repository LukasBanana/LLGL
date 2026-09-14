#version 140

layout(std140) uniform SceneView
{
    layout(row_major) mat4 cMatrix;
    layout(row_major) mat4 vpMatrix;
    layout(row_major) mat4 wMatrix;
    vec2 aspectRatio;
    float mipCount;
    float _pad0;
    vec4 lightDir;
};

uniform samplerCube s_skyBoxsmpl;

in vec4 v_VIEWRAY;
out vec4 SV_Target;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    SV_Target = texture(s_skyBoxsmpl, normalize(vec4(v_VIEWRAY.xy, gl_FrontFacing ? (-1.0) : 1.0, 0.0) * spvWorkaroundRowMajor(cMatrix)).xyz);
}

