#version 140

layout(std140) uniform Settings
{
    layout(row_major) mat4 wMatrix;
    layout(row_major) mat4 wMatrixInv;
    layout(row_major) mat4 vpMatrix;
    layout(row_major) mat4 vpMatrixInv;
    vec3 lightDir;
    float shininess;
    vec3 viewPos;
    float threshold;
    vec3 albedo;
    float reflectance;
    ivec2 viewportExtent;
};

uniform sampler3D s_noiseTexturelinearSampler;
uniform sampler2D depthRangeTexture;

in vec4 v_NDC;
in vec4 v_WORLDPOS;
in vec4 v_NORMAL;
out vec4 SV_Target;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    vec3 _71 = normalize(v_NORMAL.xyz);
    vec3 _74 = -lightDir;
    vec3 _79 = normalize(viewPos - v_WORLDPOS.xyz);
    float _80 = dot(_71, _74);
    float _85 = dot(_71, normalize(_79 + _74));
    vec3 _93 = v_WORLDPOS.xyz * 0.0500000007450580596923828125;
    vec4 _104 = texture(s_noiseTexturelinearSampler, _93 * 12.0);
    vec3 _111 = fract(((_93 * 150.0) + vec3(_104.x * 9.0)) + (_79 * 0.100000001490116119384765625));
    vec3 _113 = _111 * (vec3(1.0) - _111);
    vec4 _145 = vec4(v_NDC.xy, (texelFetch(depthRangeTexture, ivec3(int(gl_FragCoord.x), (viewportExtent.y - int(gl_FragCoord.y)) - 1, 0).xy, 0).x * 2.0) - 1.0, 1.0) * spvWorkaroundRowMajor(vpMatrixInv);
    vec3 _149 = (_145 / vec4(_145.w)).xyz;
    float _151 = distance(v_WORLDPOS.xyz, _149) * 0.078125;
    float _156;
    _156 = 0.0;
    float _153 = 0.0;
    int _158 = 0;
    for (; _158 < 64; )
    {
        vec3 _162 = vec3(_153);
        _153 += 0.015625;
        _156 += (smoothstep(0.5 - threshold, 0.5 + threshold, texture(s_noiseTexturelinearSampler, (vec4(mix(v_WORLDPOS.xyz, _149, _162), 1.0) * spvWorkaroundRowMajor(wMatrixInv)).xyz * 0.5).x) * _151);
        _158++;
        continue;
    }
    vec3 _191 = ((albedo * mix(0.20000000298023223876953125, 1.0, isnan(_80) ? 0.0 : (isnan(0.0) ? _80 : max(0.0, _80)))) * mix(0.3499999940395355224609375, 1.5, 1.0 - exp(_156 * (-0.5)))) + vec3((pow(isnan(_85) ? 0.0 : (isnan(0.0) ? _85 : max(0.0, _85)), shininess) * reflectance) + (clamp(1.0 - (7.0 * ((_113.x + _113.y) + _113.z)), 0.0, 1.0) * pow(mix(0.20000000298023223876953125, 1.0, clamp(dot(reflect(-_79, _71), _74), 0.0, 1.0)), 1.5)));
    SV_Target = vec4(_191, 1.0);
}

