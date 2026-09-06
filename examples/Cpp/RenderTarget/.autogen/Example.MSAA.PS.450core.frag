#version 450

layout(binding = 3, std140) uniform Settings
{
    layout(row_major) mat4 wvpMatrix;
    layout(row_major) mat4 wMatrix;
    vec3 lightDir;
    int useTexture2DMS;
};

layout(binding = 2) uniform sampler2D s_colorMapsamplerState;
layout(binding = 4) uniform sampler2DMS colorMapMS;

layout(location = 0) in vec3 v_NORMAL;
layout(location = 1) in vec2 v_TEXCOORD;
layout(location = 0) out vec4 SV_Target;

void main()
{
    vec4 _87;
    do
    {
        if (useTexture2DMS != 0)
        {
            uvec2 _61 = uvec2(textureSize(colorMapMS));
            uint _64 = uint(textureSamples(colorMapMS));
            ivec2 _73 = ivec2(int(v_TEXCOORD.x * float(_61.x)), int(v_TEXCOORD.y * float(_61.y)));
            vec4 _75;
            _75 = vec4(0.0);
            for (uint _78 = 0u; _78 < _64; )
            {
                _75 += texelFetch(colorMapMS, _73, int(_78));
                _78++;
                continue;
            }
            _87 = _75 / vec4(float(_64));
            break;
        }
        else
        {
            _87 = texture(s_colorMapsamplerState, v_TEXCOORD);
            break;
        }
        break; // unreachable workaround
    } while(false);
    vec3 _94 = _87.xyz * mix(0.20000000298023223876953125, 1.0, dot(lightDir, normalize(v_NORMAL)));
    SV_Target = vec4(_94.x, _94.y, _94.z, _87.w);
}

