// HLSL shader version 4.0 (for Direct3D 11/ 12)

#include <HlslToSpirvInterop.hlsli>

#define ROTATION_0  0
#define ROTATION_90 1

cbuffer SceneView : register(b2)
{
    float4x4    vpMatrix;
    float3      gridScale;
    float       pad0;
    float4      viewPos;
    float3      lightDir;
    float       specularPower; // Blinn-phong specular exponent (usual values between 16 and 90)
};

struct VertexIn
{
    // Per-vertex attributes
    float3      position    : POSITION;
    float3      normal      : NORMAL;
    
    // Per-instance attributes
    int4        gridPos     : GRIDPOS; // xyz (position), w (rotation: 0=default, 1=90 degrees)
    float4      color       : COLOR;
};

struct VertexOut
{
    float4                  position : SV_Position;
    float3                  worldPos : WORLDPOS;
    float3                  normal   : NORMAL;
    nointerpolation float3  color    : COLOR;
};


// VERTEX SHADER

void VSMain(VertexIn inp, out VertexOut outp)
{
    // Rotate model either 0 degrees (default) or 90 degrees based on the instance's gridPos.w value
    const float3x3 rotationMatrices[2] =
    {
        float3x3(
            +1.0,  0.0,  0.0,
             0.0, +1.0,  0.0,
             0.0,  0.0, +1.0
        ),
        float3x3(
             0.0,  0.0, +1.0,
             0.0, +1.0, 0.0,
            -1.0,  0.0, 0.0
        )
    };

    float3x3 rotation = rotationMatrices[inp.gridPos.w];

    // Transform vertex position from local space to world space based on instance's rotation and grid position
    outp.worldPos = mul(rotation, inp.position) + (float3)inp.gridPos.xyz * gridScale;
    outp.position = mul(vpMatrix, float4(outp.worldPos, 1.0));
    outp.normal   = mul(rotation, inp.normal);
    outp.color    = inp.color.rgb;
}


// PIXEL SHADER

float BlinnPhoneShading(float3 worldPos, float3 normal)
{
    const float ambient = 0.2;
    float3 lightVec = normalize(viewPos.xyz - lightDir.xyz);
    float3 viewDir = normalize(viewPos.xyz - worldPos);
    float3 halfVec = normalize(lightVec + viewDir);
    float NdotL = saturate(dot(normal, lightVec));
    float NdotH = saturate(dot(normal, halfVec));
    float diffuse = lerp(ambient, 1.0, NdotL);
    float specular = pow(NdotH, specularPower);
    return diffuse + specular;
}

float4 PSMain(VertexOut inp) : SV_Target
{
    // Apply simple lighting 
    float lighting = BlinnPhoneShading(inp.worldPos, normalize(inp.normal));
    float3 color = inp.color * lighting;
    return float4(color, 1.0);
};

