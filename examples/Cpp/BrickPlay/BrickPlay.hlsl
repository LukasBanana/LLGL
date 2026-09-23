// HLSL shader version 4.0 (for Direct3D 11/ 12)

cbuffer SceneView : register(b2)
{
    float4x4    vpMatrix;
    float3      gridScale;
    float       specularPower;  // Blinn-phong specular exponent (usual values between 16 and 90)
    float4      viewPos;
    float4      ambientLight;
    float4      diffuseLight[2];
    float4      lightDir[2];

    // Preview shaders only
    float       worldOffsetY;   // Y-axis for preview world offset
    int         isDithering;    // Use dithering in preview shaders

    // Outline shaders only
    float2      viewportSize;   // Viewport size in pixels; converts outlineWidth into NDC space
    float3      outlineColor;   // Color used for silhouette outlines
    float       outlineWidth;   // Thickness silhouettes outlines
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

float3x3 BrickRotation(int rotationIndex)
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
    return rotationMatrices[rotationIndex];
}

void BrickBaseTransform(VertexIn inp, out VertexOut outp, float3 worldOffset)
{
    float3x3 rotation = BrickRotation(inp.gridPos.w);

    // Transform vertex position from local space to world space based on instance's rotation and grid position
    outp.worldPos = mul(rotation, inp.position) + (float3)inp.gridPos.xyz * gridScale + worldOffset;
    outp.position = mul(vpMatrix, float4(outp.worldPos, 1.0));
    outp.normal   = mul(rotation, inp.normal);
    outp.color    = inp.color;
}

void VSBrick(VertexIn inp, out VertexOut outp)
{
    BrickBaseTransform(inp, outp, (float3)0.0);
}

float3 BlinnPhongShading(float3 worldPos, float3 normal, float3 lightVec, float3 color)
{
    float3 viewDir = normalize(viewPos.xyz - worldPos);
    float3 halfVec = normalize(lightVec + viewDir);
    float NdotL = saturate(dot(normal, lightVec));
    float NdotH = saturate(dot(normal, halfVec));
    float diffuse = NdotL;
    float specular = pow(NdotH, specularPower);
    return color * (diffuse + specular * NdotL);
}

float3 ComputeLighting(float3 worldPos, float3 normal)
{
    float3 lighting = ambientLight;

    [unroll]
    for (int i = 0; i < 2; ++i)
        lighting += BlinnPhongShading(worldPos, normal, lightDir[i].xyz, diffuseLight[i].xyz);

    return lighting;
}

float4 PSBrick(VertexOut inp) : SV_Target
{
    // Apply simple lighting 
    float3 lighting = ComputeLighting(inp.worldPos, normalize(inp.normal));
    return float4(inp.color.rgb * lighting, inp.color.a);
};


// PREVIEW SHADERS

void VSBrickPreview(VertexIn inp, out VertexOut outp)
{
    BrickBaseTransform(inp, outp, float3(0.0, worldOffsetY, 0.0));
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
    float3 lighting = ComputeLighting(inp.worldPos, normalize(inp.normal));
    return float4(inp.color.rgb * lighting, inp.color.a);
};


// SILHOUETTE SHADERS

struct OutlineVertexOut
{
    float4 position : SV_Position;
};

void VSBrickOutline(VertexIn inp, out OutlineVertexOut outp)
{
    float3x3 rotation = BrickRotation(inp.gridPos.w);

    // Transform vertex position from local space to world space based on instance's rotation and grid position
    float3 worldPos = mul(rotation, inp.position) + (float3)inp.gridPos.xyz * gridScale;
    outp.position = mul(vpMatrix, float4(worldPos, 1.0));
}

// Transforms a screen space position (in pixels, relative to the viewport center) back into clip space
OutlineVertexOut MakeOutlineVertex(float2 screenPos, float clipZ, float clipW)
{
    OutlineVertexOut outp;
    outp.position = float4(screenPos * (2.0 / viewportSize) * clipW, clipZ, clipW);
    return outp;
}

// Worst case is 27 vertices that turned 1 triangle into 9 triangles for a thick outline
[maxvertexcount(27)]
void GSBrickOutline(triangle OutlineVertexOut inp[3], inout TriangleStream<OutlineVertexOut> streamOut)
{
    // Discard triangles that cross the near clipping plane as their screen space projection is undefined
    if (inp[0].position.w <= 0.0 || inp[1].position.w <= 0.0 || inp[2].position.w <= 0.0)
        return;

    // Project all corners into screen space (in pixels) to make the line thickness resolution independent
    float2 halfRes = viewportSize * 0.5;
    float2 corner[3];

    [unroll]
    for (uint i = 0; i < 3; ++i)
        corner[i] = inp[i].position.xy / inp[i].position.w * halfRes;

    // Build the extrusion vectors for all three edges, i.e. the edge normals scaled to half the line thickness
    float2 edgeDir[3];
    float2 edgeExt[3];

    [unroll]
    for (uint j = 0; j < 3; ++j)
    {
        float2 edgeVec = corner[(j + 1) % 3] - corner[j];
        float edgeLen = length(edgeVec);
        edgeDir[j] = (edgeLen > 1e-6 ? edgeVec / edgeLen : float2(1.0, 0.0));
        edgeExt[j] = float2(-edgeDir[j].y, edgeDir[j].x) * (outlineWidth * 0.5);
    }

    // Emit one quad (2 triangles) per edge, extruded symmetrically to both sides of the edge
    [unroll]
    for (uint k = 0; k < 3; ++k)
    {
        uint k1 = (k + 1) % 3;
        float4 p0 = inp[k].position;
        float4 p1 = inp[k1].position;

        streamOut.Append(MakeOutlineVertex(corner[k ] - edgeExt[k], p0.z, p0.w));
        streamOut.Append(MakeOutlineVertex(corner[k ] + edgeExt[k], p0.z, p0.w));
        streamOut.Append(MakeOutlineVertex(corner[k1] - edgeExt[k], p1.z, p1.w));
        streamOut.Append(MakeOutlineVertex(corner[k1] + edgeExt[k], p1.z, p1.w));
        streamOut.RestartStrip();
    }

    // Close the gap on the outer side of each corner with a bevel joint.
    // A miter joint would be extruded arbitrarily far for sharp angles, so an extra triangle is emitted instead.
    [unroll]
    for (uint n = 0; n < 3; ++n)
    {
        uint nPrev = (n + 2) % 3;

        // Cross product of both edge directions determines which side of the corner the gap is on
        float turn = edgeDir[nPrev].x*edgeDir[n].y - edgeDir[nPrev].y*edgeDir[n].x;
        float side = (turn > 0.0 ? -1.0 : +1.0);

        float4 p = inp[n].position;
        OutlineVertexOut center = MakeOutlineVertex(corner[n], p.z, p.w);
        OutlineVertexOut joint0 = MakeOutlineVertex(corner[n] + edgeExt[nPrev]*side, p.z, p.w);
        OutlineVertexOut joint1 = MakeOutlineVertex(corner[n] + edgeExt[n    ]*side, p.z, p.w);

        // Swap the outer vertices to keep the winding order consistent with the edge quads
        streamOut.Append(center);
        if (side > 0.0)
        {
            streamOut.Append(joint0);
            streamOut.Append(joint1);
        }
        else
        {
            streamOut.Append(joint1);
            streamOut.Append(joint0);
        }
        streamOut.RestartStrip();
    }
}

float4 PSBrickOutline() : SV_Target
{
    return float4(outlineColor, 1);
}


// DEBUG SHADERS

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


// UI SHADERS

cbuffer UIView : register(b1)
{
    float4x4 pMatrix;
}

struct UIVertexIn
{
    int2    position    : POSITION;
    uint4   texCoord    : TEXCOORD;
    float4  color       : COLOR;
};

struct UIVertexOut
{
    float4                  position    : SV_Position;
    float3                  texCoord    : TEXCOORD;
    float4                  color       : COLOR;
    nointerpolation bool    isTextured  : IS_TEXTURED;
};

void VSOverlay(UIVertexIn inp, out UIVertexOut outp)
{
    outp.position   = mul(pMatrix, float4((float2)inp.position.xy, 0.0, 1.0));
    outp.color      = inp.color;

    if (inp.texCoord.w != 0)
    {
        outp.texCoord   = (float3)inp.texCoord.xyz;
        outp.isTextured = true;
    }
    else
    {
        outp.texCoord   = float3(0.0, 0.0, 0.0);
        outp.isTextured = false;
    }
}

Texture2DArray uiTexture : register(t2);
SamplerState uiTextureSampler : register(s3);

float4 PSOverlay(UIVertexOut inp) : SV_Target
{
    if (inp.isTextured)
        return uiTexture.Sample(uiTextureSampler, inp.texCoord) * inp.color;
    else
        return inp.color;
}

