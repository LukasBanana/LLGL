#version 430

struct type_PushConstant_ModelData
{
    vec3 lightVec;
    uint instance;
};

uniform type_PushConstant_ModelData model;

layout(binding = 4) uniform sampler2D s_colorMapcolorMapSampler;

layout(location = 1) in vec3 v_NORMAL;
layout(location = 2) in vec2 v_TEXCOORD;
layout(location = 0) out vec4 SV_Target;

void main()
{
    vec4 _36 = texture(s_colorMapcolorMapSampler, v_TEXCOORD);
    float _40 = dot(normalize(v_NORMAL), model.lightVec);
    SV_Target = vec4(_36.xyz * (isnan(_40) ? 0.20000000298023223876953125 : (isnan(0.20000000298023223876953125) ? _40 : max(0.20000000298023223876953125, _40))), _36.w);
}

