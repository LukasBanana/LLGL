#version 140

layout(std140) uniform Scene
{
    layout(row_major) mat4 wvpMatrix;
    layout(row_major) mat4 wMatrix;
    vec3 lightVec;
};

uniform sampler2D s_colorMapsamplerState;

in vec3 v_NORMAL;
in vec2 v_TEXCOORD;
out vec4 SV_Target;

void main()
{
    vec4 _37 = texture(s_colorMapsamplerState, v_TEXCOORD);
    vec3 _44 = _37.xyz * mix(0.20000000298023223876953125, 1.0, dot(lightVec, normalize(v_NORMAL)));
    SV_Target = vec4(_44.x, _44.y, _44.z, _37.w);
}

