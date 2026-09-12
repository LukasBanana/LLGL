#version 140

layout(std140) uniform SceneView
{
    layout(row_major) mat4 wvpMatrix;
    layout(row_major) mat4 wMatrix;
    vec4 lightVec;
    vec4 baseColor;
};

struct type_PushConstant_DynamicState_t
{
    float texCoordScaleFront;
    float texCoordScaleBack;
    float interpolationFactor;
    float invertXAxis;
    int borderSamplerFront;
    int borderSamplerBack;
};

uniform type_PushConstant_DynamicState_t dynamicState;

uniform sampler2D s_frontPageColorMapfrontPageSampler;
uniform sampler2D s_backPageColorMapbackPageSampler;
uniform sampler2D s_paperDetailMappaperDetailMapSampler;

in vec3 v_NORMAL;
in vec2 v_TEXCOORD;
flat in float v_FLIPFACE;
out vec4 SV_Target;

void main()
{
    vec2 _74;
    if (gl_FrontFacing)
    {
        _74 = ((v_TEXCOORD - vec2(0.5)) * dynamicState.texCoordScaleFront) + vec2(0.5);
    }
    else
    {
        _74 = ((vec2(1.0 - v_TEXCOORD.x, v_TEXCOORD.y) - vec2(0.5)) * dynamicState.texCoordScaleBack) + vec2(0.5);
    }
    vec4 _86;
    if (gl_FrontFacing)
    {
        _86 = texture(s_frontPageColorMapfrontPageSampler, _74);
    }
    else
    {
        _86 = texture(s_backPageColorMapbackPageSampler, _74);
    }
    vec4 _119 = (baseColor + vec4(texture(s_paperDetailMappaperDetailMapSampler, _74).xyz - vec3(0.5), 0.0)) * vec4(mix(baseColor.xyz, _86.xyz, vec3(_86.w)), 1.0);
    SV_Target = vec4(_119.xyz * mix(0.20000000298023223876953125, 1.0, dot(lightVec.xyz, normalize(v_NORMAL * (v_FLIPFACE * mix(-1.0, 1.0, float(gl_FrontFacing)))))), _119.w);
}

