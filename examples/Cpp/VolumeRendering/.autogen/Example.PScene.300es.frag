#version 300 es
precision mediump float;
precision highp int;

layout(std140) uniform Settings
{
    layout(row_major) highp mat4 wMatrix;
    layout(row_major) highp mat4 wMatrixInv;
    layout(row_major) highp mat4 vpMatrix;
    layout(row_major) highp mat4 vpMatrixInv;
    highp vec3 lightDir;
    highp float shininess;
    highp vec3 viewPos;
    highp float threshold;
    highp vec3 albedo;
    highp float reflectance;
    ivec2 viewportExtent;
};

uniform highp sampler3D s_noiseTexturelinearSampler;
uniform highp sampler2D depthRangeTexture;

in highp vec4 v_NDC;
in highp vec4 v_WORLDPOS;
in highp vec4 v_NORMAL;
layout(location = 0) out highp vec4 SV_Target;

highp mat4 spvWorkaroundRowMajor(highp mat4 wrap) { return wrap; }
mediump mat4 spvWorkaroundRowMajorMP(mediump mat4 wrap) { return wrap; }

void main()
{
    highp vec3 _71 = normalize(v_NORMAL.xyz);
    highp vec3 _74 = -lightDir;
    highp vec3 _79 = normalize(viewPos - v_WORLDPOS.xyz);
    highp float _80 = dot(_71, _74);
    highp float _85 = dot(_71, normalize(_79 + _74));
    highp vec3 _93 = v_WORLDPOS.xyz * 0.0500000007450580596923828125;
    highp vec4 _104 = texture(s_noiseTexturelinearSampler, _93 * 12.0);
    highp vec3 _111 = fract(((_93 * 150.0) + vec3(_104.x * 9.0)) + (_79 * 0.100000001490116119384765625));
    highp vec3 _113 = _111 * (vec3(1.0) - _111);
    highp vec4 _145 = vec4(v_NDC.xy, (texelFetch(depthRangeTexture, ivec3(int(gl_FragCoord.x), (viewportExtent.y - int(gl_FragCoord.y)) - 1, 0).xy, 0).x * 2.0) - 1.0, 1.0) * spvWorkaroundRowMajor(vpMatrixInv);
    highp vec3 _149 = (_145 / vec4(_145.w)).xyz;
    highp float _151 = distance(v_WORLDPOS.xyz, _149) * 0.078125;
    highp float _156;
    _156 = 0.0;
    highp float _153 = 0.0;
    int _158 = 0;
    for (; _158 < 64; )
    {
        highp vec3 _162 = vec3(_153);
        _153 += 0.015625;
        _156 += (smoothstep(0.5 - threshold, 0.5 + threshold, texture(s_noiseTexturelinearSampler, (vec4(mix(v_WORLDPOS.xyz, _149, _162), 1.0) * spvWorkaroundRowMajor(wMatrixInv)).xyz * 0.5).x) * _151);
        _158++;
        continue;
    }
    highp vec3 _191 = ((albedo * mix(0.20000000298023223876953125, 1.0, isnan(_80) ? 0.0 : (isnan(0.0) ? _80 : max(0.0, _80)))) * mix(0.3499999940395355224609375, 1.5, 1.0 - exp(_156 * (-0.5)))) + vec3((pow(isnan(_85) ? 0.0 : (isnan(0.0) ? _85 : max(0.0, _85)), shininess) * reflectance) + (clamp(1.0 - (7.0 * ((_113.x + _113.y) + _113.z)), 0.0, 1.0) * pow(mix(0.20000000298023223876953125, 1.0, clamp(dot(reflect(-_79, _71), _74), 0.0, 1.0)), 1.5)));
    SV_Target = vec4(_191, 1.0);
}

