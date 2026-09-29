/*
 * BrickPlay.hlsl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef MAX_NUM_PARALLAX_LAYERS
#define MAX_NUM_PARALLAX_LAYERS 16
#endif

#ifndef MAX_NUM_COLOR_PALETTE
#define MAX_NUM_COLOR_PALETTE 32
#endif

#ifndef ENABLE_PARALLAX_OCCLUSION
#define ENABLE_PARALLAX_OCCLUSION 0
#endif

#ifndef COMPILE_FOR_GL
#define COMPILE_FOR_GL 0
#endif

// For transparent objects, flip the normal vector to simulate light passing through and shading the face from the opposite side
#ifndef ENABLE_DUAL_SIDED_LIGHTING
#define ENABLE_DUAL_SIDED_LIGHTING 0
#endif

// Disable early depth-stencil for OpenGL, as it it required higher GLSL versions than 140.
#if COMPILE_FOR_GL
#define EARLYDEPTHSTENCIL
#else
#define EARLYDEPTHSTENCIL [earlydepthstencil]
#endif

// General purpose scene view constant buffer. Shared across all shaders in this file.
cbuffer SceneView : register(b2)
{
    float4x4    vpMatrix;
    float4x4    vpMatrixInv;            // Inverse view-projection matrix to displace depth values for parallax occlusion displacement
    float4x4    vpMatrixInvNoViewPos;   // Inverse view-projection matrix without view position to transform a view ray
    float3      gridScale;
    float       specularPower;      // Blinn-phong specular exponent (usual values between 16 and 90)
    float3      viewPos;
    float       skyboxDiffuseLOD;       // Maximum level of detail (LOD) for the skybox's diffuse reflection (default to 6.0)
    float4      ambientLight;
    float4      diffuseLight[2];
    float4      lightDir[2];

    // Preview shaders only
    float       worldOffsetY;       // Y-axis for preview world offset
    int         isDithering;        // Use dithering in preview shaders

    // Outline shaders only
    float2      viewportSize;       // Viewport size in pixels; converts outlineWidth into NDC space
    float3      outlineColor;       // Color used for silhouette outlines
    float       outlineWidth;       // Thickness silhouettes outlines

    // Plane shaders only
    int4        gridBounds;             // XY: minimum grid bounds, ZW: maximum grid bounds (negated for easier comparison)
    float4      planeBaseColors[2];
    float2      parallaxFadeRange;      // Range for parallax fade effect
    float       studHeight;             // Height of the studs on the bricks

    float       skyboxDiffuseFactor;    // Factor for blending the skybox's diffuse reflection
};

cbuffer ColorPalette : register(b3)
{
    uint4 colorPalette[(MAX_NUM_COLOR_PALETTE + 3) / 4];
};

TextureCube skyboxTexture : register(t7);
SamplerState skyboxSampler : register(s8);


////////////////////// PRIMARY INSTANCE SHADERS //////////////////////

struct VertexIn
{
    // Per-vertex attributes
    float3      position        : POSITION;
    float3      normal          : NORMAL;
    
    // Per-instance attributes
    int4        packedInstance  : PACKEDINSTANCE; // xyz (position), w[31:16] (rotation: 0=default, 1=90 degrees), w[15:0] (color index)
};

struct VertexOut
{
    float4                  position : SV_Position;
    float3                  worldPos : WORLDPOS;
    float3                  normal   : NORMAL;
    nointerpolation float4  color    : COLOR;
};

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

float4 UnpackColorRGBA8UNorm(uint packedColor)
{
    uint4 color;
    color.r = ((packedColor      ) & 0xFF);
    color.g = ((packedColor >>  8) & 0xFF);
    color.b = ((packedColor >> 16) & 0xFF);
    color.a = ( packedColor >> 24  & 0xFF);
    return (float4)color / 255.0;
}

float4 ColorFromPalette(uint colorIndex)
{
    uint packedColor = colorPalette[colorIndex / 4][colorIndex % 4];
    return UnpackColorRGBA8UNorm(packedColor);
}

void BrickBaseTransform(VertexIn inp, out VertexOut outp, float3 worldOffset)
{
    // Unpack W component from packed instanced data
    uint packedInstanceW = asuint(inp.packedInstance.w);

    int3 gridPos = inp.packedInstance.xyz;
    uint rotationMode = packedInstanceW & 0xFF;
    uint colorIndex = packedInstanceW >> 8;

    float3x3 rotation = BrickRotation(rotationMode);

    // Transform vertex position from local space to world space based on instance's rotation and grid position
    outp.worldPos = mul(rotation, inp.position) + (float3)gridPos * gridScale + worldOffset;
    outp.position = mul(vpMatrix, float4(outp.worldPos, 1.0));
    outp.normal   = mul(rotation, inp.normal);
    outp.color    = ColorFromPalette(colorIndex);
}

void VSBrick(VertexIn inp, out VertexOut outp)
{
    BrickBaseTransform(inp, outp, (float3)0.0);
}

float3 BlinnPhongShading(float3 worldPos, float3 normal, float3 lightVec, float3 color)
{
    #if ENABLE_DUAL_SIDED_LIGHTING
    normal = lerp(-normal, normal, step(0.0, dot(normal, lightVec)));
    #endif

    // Compute light reflection vectors
    float3 viewDir = normalize(viewPos - worldPos);
    float3 halfVec = normalize(lightVec + viewDir);
    float NdotL = saturate(dot(normal, lightVec));
    float NdotH = saturate(dot(normal, halfVec));

    // Compute diffuse and specular terms
    float diffuse = NdotL;
    float specular = pow(NdotH, specularPower) * NdotL;

    // Sample environment map for specular highlights
    float3 reflectVec = reflect(-viewDir, normal);
    float envmapLOD = skyboxDiffuseLOD * (1.0 - specular);
    float3 envmapReflection = skyboxTexture.SampleBias(skyboxSampler, reflectVec, envmapLOD).rgb;

    // Compute final shading
    return color * (diffuse * lerp(1.0, envmapReflection, 0.2) + specular);
}

float3 ComputeLighting(float3 worldPos, float3 normal)
{
    float3 lighting = ambientLight.rgb;

    [unroll]
    for (int i = 0; i < 2; ++i)
        lighting += BlinnPhongShading(worldPos, normal, lightDir[i].xyz, diffuseLight[i].xyz);

    return lighting;
}

EARLYDEPTHSTENCIL
float4 PSBrick(VertexOut inp) : SV_Target
{
    // Apply simple lighting
    float3 lighting = ComputeLighting(inp.worldPos, normalize(inp.normal));
    return float4(inp.color.rgb * lighting, inp.color.a);
}

struct TransparentPixelOut
{
    float4 accum  : SV_Target0;
    float  reveal : SV_Target1;
};

float Max3(float a, float b, float c)
{
    return max(a, max(b, c));
}

float ComputeWeight(float4 color, float z)
{
    // This formula has been adopted straight from https://learnopengl.com/Guest-Articles/2020/OIT/Weighted-Blended
    // I solemnly swear, I have no idea what this does :o)
    return
    (
        max(min(1.0, Max3(color.r, color.g, color.b) * color.a), color.a) *
        clamp(0.03 / (1e-5 + pow(z / 200.0, 4.0)), 1e-2, 3e3)
    );
}

// Adopted from https://learnopengl.com/Guest-Articles/2020/OIT/Weighted-Blended
EARLYDEPTHSTENCIL
void PSBrickTransparent(VertexOut inp, out TransparentPixelOut outp)
{
    // Apply simple lighting
    float3 lighting = ComputeLighting(inp.worldPos, normalize(inp.normal));
    float4 outputColor = float4(inp.color.rgb * lighting, inp.color.a);

    // Output weighted color to accumulation buffer
    float weight = ComputeWeight(outputColor, inp.position.z);

    outp.accum = float4(outputColor.rgb * outputColor.a, outputColor.a) * weight;
    outp.reveal = outputColor.a;
}


////////////////////// TRANSPARENCY COMPOSITION SHADERS //////////////////////

Texture2D<float3> colorTexture : register(t2);
Texture2D<float4> accumTexture : register(t3);
Texture2D<float> revealTexture : register(t4);

/*
This function generates the coordinates for a fullscreen triangle with its vertex IDs:

(-1,+3)
   *
   | \
   |   \
   |     \
   |       \
(-1,+1)    (+1,+1)
   *----------*\
   |          |  \
   |  screen  |    \
   |          |      \
   *----------*--------*
(-1,-1)    (+1,-1)  (+3,-1)
*/
float4 GetFullscreenTriangleVertex(uint id)
{
    const float v[4] = { -1.0, +3.0, -1.0, -1.0 };
    return float4(v[id], v[id + 1], 1.0, 1.0);
}

float4 VSTransparentComposite(uint id : SV_VertexID) : SV_Position
{
    return GetFullscreenTriangleVertex(id);
}

float4 PSTransparentComposite(float4 screenPos : SV_Position) : SV_Target
{
    // Fetch framebuffer information
    int3 texCoord = int3(screenPos.xy, 0);

    #if COMPILE_FOR_GL
    // `layout(origin_upper_left)` is not supported in ESSL 300, so just query the texture dimensions to flip the Y-axis in GL backend.
    // This can be optimized by using `viewportSize.y` instead, but to keep the C++ code simple across all backends, we use this method.
    uint2 colorTextureSize = (uint2)0;
    colorTexture.GetDimensions(colorTextureSize.x, colorTextureSize.y);
    texCoord.y = (int)colorTextureSize.y - texCoord.y - 1;
    #endif

    float3 color = colorTexture.Load(texCoord);
    float4 accum = accumTexture.Load(texCoord);
    float reveal = revealTexture.Load(texCoord);

    // Perform custom alpha blending with common SrcAlpha/InvSrcAlpha scheme
    return float4(lerp(accum.rgb / max(accum.a, 1e-5), color, reveal), 1.0);
}


////////////////////// PREVIEW SHADERS //////////////////////

void VSBrickPreview(VertexIn inp, out VertexOut outp)
{
    BrickBaseTransform(inp, outp, float3(0.0, worldOffsetY, 0.0));
    outp.worldPos.y += worldOffsetY;
    outp.position = mul(vpMatrix, float4(outp.worldPos, 1.0));
}

EARLYDEPTHSTENCIL
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

////////////////////// SILHOUETTE SHADERS //////////////////////

struct OutlineVertexOut
{
    float4 position : SV_Position;
};

void VSBrickOutline(VertexIn inp, out OutlineVertexOut outp)
{
    // Unpack W component from packed instanced data
    uint packedInstanceW = asuint(inp.packedInstance.w);

    int3 gridPos = inp.packedInstance.xyz;
    uint rotationMode = packedInstanceW & 0xFF;

    float3x3 rotation = BrickRotation(rotationMode);

    // Transform vertex position from local space to world space based on instance's rotation and grid position
    float3 worldPos = mul(rotation, inp.position) + (float3)gridPos * gridScale;
    outp.position = mul(vpMatrix, float4(worldPos, 1.0));
}

// Transforms a screen space position (in pixels, relative to the viewport center) back into clip space
OutlineVertexOut MakeOutlineVertex(float2 screenPos, float clipZ, float clipW)
{
    OutlineVertexOut outp;
    outp.position = float4(screenPos * (2.0 / viewportSize) * clipW, clipZ, clipW);
    return outp;
}

// This geometry shader turns a single triangle into a wireframe version of itself with thick lines, each such line being represented by two triangles.
// Sharp corners are filled with another triangle. Worst case is 27 vertices that turned 1 triangle into 9 triangles.
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


////////////////////// PLANE SHADERS //////////////////////

struct PlaneVertexIn
{
    float3 worldPos : WORLDPOS;
};

struct PlaneVertexOut
{
    float4 position : SV_Position;
    float3 worldPos : WORLDPOS;
};

void VSPlane(PlaneVertexIn inp, out PlaneVertexOut outp)
{
    outp.position = mul(vpMatrix, float4(inp.worldPos, 1.0));
    outp.worldPos = inp.worldPos;
}

SamplerState planeSampler : register(s3);
Texture2D<float4> planeColorMap : register(t4);
Texture2D<float3> planeNormalMap : register(t5);

float3 VectorToPlaneTangetSpace(float3 v)
{
    return float3(v.x, v.z, -v.y);
}

float3 ColorToPlaneNormal(float3 color)
{
    return VectorToPlaneTangetSpace(color * 2.0 - 1.0);
}

#if ENABLE_PARALLAX_OCCLUSION

Texture2D<float> planeHeightMap : register(t6);

float SampleHeightMap(float2 texCoord, float2 dx, float2 dy)
{
    return planeHeightMap.SampleGrad(planeSampler, texCoord, dx, dy);
}

// Adopted from https://learnopengl.com/Advanced-Lighting/Parallax-Mapping
float3 ParallaxOffset(float2 texCoord, float3 viewDir, float2 dx, float2 dy)
{
    const float numLayers = (float)MAX_NUM_PARALLAX_LAYERS;

    // Parameters for stepping through the layers of steep parallax mapping
    float layerDepth = 1.0 / numLayers;
    float currentLayerDepth = 1.0;

    // Amount of shift texture coordinates per layer
    float2 offset = viewDir.xy / viewDir.z * studHeight;
    float2 deltaTexCoords = offset * layerDepth;

    // Initial values
    float2 currentTexCoords = texCoord - offset;
    float currentHeight = SampleHeightMap(currentTexCoords, dx, dy);

    [unroll(MAX_NUM_PARALLAX_LAYERS)]
    while (currentLayerDepth > currentHeight)
    {
        currentTexCoords += deltaTexCoords;
        currentHeight = SampleHeightMap(currentTexCoords, dx, dy);
        currentLayerDepth -= layerDepth;
    }

    // Write out final texture coordinates and height (in world units)
    return float3(currentTexCoords, studHeight * currentHeight);
}

// Returns displaced texture offset (XY) and height (Z)
float3 ParallaxOffsetWithFade(float2 texCoord, float3 viewDir, float viewDist, float2 dx, float2 dy)
{
    // Force branching to ensure pixels with far viewing distance do not perform unnecessary parallax calculations
    [branch]
    if (viewDist < parallaxFadeRange[1])
    {
        float fade = saturate((viewDist - parallaxFadeRange[0]) / (parallaxFadeRange[1] - parallaxFadeRange[0]));
        float3 parallaxTexCoord = ParallaxOffset(texCoord, viewDir, dx, dy);
        return lerp(parallaxTexCoord, float3(texCoord, 0.0), fade);
    }
    else
    {
        return float3(texCoord, 0.0);
    }
}

// Computes the displaced depth for the current fragment based on the parallax occlusion mapping.
float ComputeDisplacedDepth(float3 texCoordAndHeight)
{
    float4 clipPos = mul(vpMatrix, float4(texCoordAndHeight.xzy, 1.0));
    #if COMPILE_FOR_GL
    // Adjust for OpenGL's unit cube depth range
    return (clipPos.z / clipPos.w) * 0.5 + 0.5;
    #else
    return clipPos.z / clipPos.w;
    #endif
}

#endif // /ENABLE_PARALLAX_OCCLUSION

struct PlanePixelOut
{
    float4 color : SV_Target;
    #if ENABLE_PARALLAX_OCCLUSION
    float  depth : SV_Depth;
    #endif
};

#if !ENABLE_PARALLAX_OCCLUSION
EARLYDEPTHSTENCIL
#endif
void PSPlane(PlaneVertexOut inp, out PlanePixelOut outp)
{
    // Apply parallex occlusion displacement
    float2 texCoord = inp.worldPos.xz;
    float2 dx = ddx(texCoord);
    float2 dy = ddy(texCoord);

    float viewDist = length(viewPos - inp.worldPos.xyz);

    #if ENABLE_PARALLAX_OCCLUSION
    float3 viewDir = VectorToPlaneTangetSpace(normalize(viewPos - inp.worldPos.xyz));
    float3 texCoordAndHeight = ParallaxOffsetWithFade(texCoord, viewDir, viewDist, dx, dy);
    #else
    float2 texCoordAndHeight = texCoord;
    #endif

    float3 planeNormal = ColorToPlaneNormal(planeNormalMap.SampleGrad(planeSampler, texCoordAndHeight.xy, dx, dy));

    // Apply tint for marking the bounds of the grid
    int2 gridPos = (int2)texCoordAndHeight.xy;
    bool isOutsideGrid = any(int4(gridPos, -gridPos) <= gridBounds);
    float4 baseColor = lerp(planeBaseColors[0], planeBaseColors[1], isOutsideGrid);

    // Sample plane color map
    float4 color = planeColorMap.SampleGrad(planeSampler, texCoordAndHeight.xy, dx, dy) * baseColor;

    // Apply simple lighting
    float3 lighting = ComputeLighting(inp.worldPos, normalize(planeNormal));
    outp.color = float4(color.rgb * lighting, color.a);

    #if ENABLE_PARALLAX_OCCLUSION
    // Back project depth to world position, apply displacement, then project depth back into clip space
    outp.depth = ComputeDisplacedDepth(texCoordAndHeight);
    #endif
}


////////////////////// SKYBOX SHADERS //////////////////////

struct SkyVertexOut
{
    float4 position : SV_Position;
    float4 viewRay : VIEWRAY;
};

void VSSkybox(uint id : SV_VertexID, out SkyVertexOut outp)
{
    outp.position = GetFullscreenTriangleVertex(id);
    outp.viewRay = mul(vpMatrixInvNoViewPos, float4(outp.position.xy, 0.0, 1.0));
    outp.viewRay /= outp.viewRay.w;
}

float4 PSSkybox(SkyVertexOut inp) : SV_Target
{
    return skyboxTexture.Sample(skyboxSampler, normalize(inp.viewRay.xyz));
}


////////////////////// UI SHADERS //////////////////////

#define UI_MODE_SOLID       0
#define UI_MODE_DITHERED    1
#define UI_MODE_TEXTURED    2

cbuffer UIView : register(b1)
{
    float4x4 pMatrix;
}

struct UIVertexIn
{
    int2    position        : POSITION;
    uint4   texCoordAndMode : TEXCOORDANDMODE;
    float4  color           : COLOR;
};

struct UIVertexOut
{
    float4                  position    : SV_Position;
    float3                  texCoord    : TEXCOORD;
    float4                  color       : COLOR;
    nointerpolation uint    mode        : MODE;
};

void VSOverlay(UIVertexIn inp, out UIVertexOut outp)
{
    outp.position   = mul(pMatrix, float4((float2)inp.position.xy, 0.0, 1.0));
    outp.texCoord   = (float3)inp.texCoordAndMode.xyz;
    outp.color      = inp.color;
    outp.mode       = inp.texCoordAndMode.w;
}

Texture2DArray uiTexture : register(t2);
SamplerState uiTextureSampler : register(s3);

static bool DitheredPattern(uint2 screenPos)
{
    const uint patternSize = 2;
    //return (screenPos.x % 4 >= 2) != (screenPos.y % 4 >= 2);
    return (screenPos.x % (patternSize*2) >= patternSize) != (screenPos.y % (patternSize*2) >= patternSize);
}

float4 PSOverlay(UIVertexOut inp) : SV_Target
{
    switch (inp.mode)
    {
        case UI_MODE_TEXTURED:
            return uiTexture.Sample(uiTextureSampler, inp.texCoord) * inp.color;
        case UI_MODE_DITHERED:
            return lerp(inp.color, (float4)0.0, DitheredPattern((uint2)inp.position.xy));
        case UI_MODE_SOLID:
        default:
            return inp.color;
    }
}

