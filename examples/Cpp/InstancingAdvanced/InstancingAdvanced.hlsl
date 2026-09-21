// HLSL shader version 4.0 (for Direct3D 11/ 12)

cbuffer SceneView : register(b2)
{
    float4x4    vpMatrix;
    float3      gridScale;
    float       pad0;
    float4      viewPos;
    float3      lightDir;
    float       specularPower;  // Blinn-phong specular exponent (usual values between 16 and 90)

    // Preview shaders only
    float       worldOffsetY;   // Y-axis for preview world offset
    int         isDithering;    // Use dithering in preview shaders
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
    nointerpolation float4  color    : COLOR;
};


// PRIMARY INSTANCE SHADERS

void VSBrickBase(VertexIn inp, out VertexOut outp, float3 worldOffset)
{
    // Rotate model either 0 degrees (default) or 90 degrees based on the instance's gridPos.w value
    const float3x3 rotationMatrices[4] =
    {
        float3x3(
            +1.0,  0.0,  0.0,
             0.0, +1.0,  0.0,
             0.0,  0.0, +1.0
        ),
        float3x3(
             0.0,  0.0, -1.0,
             0.0, +1.0,  0.0,
            +1.0,  0.0,  0.0
        ),
        float3x3(
            -1.0,  0.0,  0.0,
             0.0, +1.0,  0.0,
             0.0,  0.0, -1.0
        ),
        float3x3(
             0.0,  0.0, +1.0,
             0.0, +1.0,  0.0,
            -1.0,  0.0,  0.0
        )
    };

    float3x3 rotation = rotationMatrices[inp.gridPos.w];

    // Transform vertex position from local space to world space based on instance's rotation and grid position
    outp.worldPos = mul(rotation, inp.position) + (float3)inp.gridPos.xyz * gridScale + worldOffset;
    outp.position = mul(vpMatrix, float4(outp.worldPos, 1.0));
    outp.normal   = mul(rotation, inp.normal);
    outp.color    = inp.color;
}

void VSBrick(VertexIn inp, out VertexOut outp)
{
    VSBrickBase(inp, outp, (float3)0.0);
}

float BlinnPhongShading(float3 worldPos, float3 normal)
{
    const float ambient = 0.2;
    float3 lightVec = normalize(lightDir.xyz);
    float3 viewDir = normalize(viewPos.xyz - worldPos);
    float3 halfVec = normalize(lightVec + viewDir);
    float NdotL = saturate(dot(normal, lightVec));
    float NdotH = saturate(dot(normal, halfVec));
    float diffuse = lerp(ambient, 1.0, NdotL);
    float specular = pow(NdotH, specularPower);
    return diffuse + specular;
}

float4 PSBrick(VertexOut inp) : SV_Target
{
    // Apply simple lighting 
    float lighting = BlinnPhongShading(inp.worldPos, normalize(inp.normal));
    return float4(inp.color.rgb * lighting, inp.color.a);
};


// PREVIEW SHADERS

void VSBrickPreview(VertexIn inp, out VertexOut outp)
{
    VSBrickBase(inp, outp, float3(0.0, worldOffsetY, 0.0));
    outp.worldPos.y += worldOffsetY;
    outp.position = mul(vpMatrix, float4(outp.worldPos, 1.0));
}

float4 PSBrickPreview(VertexOut inp) : SV_Target
{
    if (isDithering != 0)
    {
        uint2 screenPos = (uint2)inp.position.xy;
        clip((screenPos.x % 4 >= 2) != (screenPos.y % 4 >= 2) ? -1.0 : +1.0);
    }

    // Apply simple lighting 
    float lighting = BlinnPhongShading(inp.worldPos, normalize(inp.normal));
    return float4(inp.color.rgb * lighting, inp.color.a);
};


// DEBUG SHADER

uniform float4 vertexA;
uniform float4 vertexB;

float4 VSDebug(uint id : SV_VertexID) : SV_Position
{
    float4 vertices[2] = { vertexA, vertexB };
    return mul(vpMatrix, vertices[id]);
}

float4 PSDebug() : SV_Target
{
    return float4(1, 1, 1, 1);
}


