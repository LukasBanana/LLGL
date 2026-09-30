#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Settings
{
    float4x4 wvpMatrix;
    float4x4 wMatrix;
    packed_float3 lightDir;
    int useTexture2DMS;
};

struct PS_out
{
    float4 out_var_SV_Target [[color(0)]];
};

struct PS_in
{
    float3 in_var_NORMAL [[user(locn0)]];
    float2 in_var_TEXCOORD [[user(locn1)]];
};

fragment PS_out PS(PS_in in [[stage_in]], constant type_Settings& Settings [[buffer(3)]], texture2d<float> colorMap [[texture(2)]], texture2d_ms<float> colorMapMS [[texture(4)]], sampler samplerState [[sampler(1)]])
{
    PS_out out = {};
    float4 _87;
    do
    {
        if (Settings.useTexture2DMS != 0)
        {
            uint2 _61 = uint2(colorMapMS.get_width(), colorMapMS.get_height());
            uint _64 = uint(colorMapMS.get_num_samples());
            int2 _73 = int2(int(in.in_var_TEXCOORD.x * float(_61.x)), int(in.in_var_TEXCOORD.y * float(_61.y)));
            float4 _75;
            _75 = float4(0.0);
            for (uint _78 = 0u; _78 < _64; )
            {
                _75 += colorMapMS.read(uint2(_73), int(_78));
                _78++;
                continue;
            }
            _87 = _75 / float4(float(_64));
            break;
        }
        else
        {
            _87 = colorMap.sample(samplerState, in.in_var_TEXCOORD);
            break;
        }
        break; // unreachable workaround
    } while(false);
    float3 _94 = _87.xyz * mix(0.20000000298023223876953125, 1.0, dot(float3(Settings.lightDir), fast::normalize(in.in_var_NORMAL)));
    out.out_var_SV_Target = float4(_94.x, _94.y, _94.z, _87.w);
    return out;
}

