// Metal tessellation shader

#include <metal_stdlib>

using namespace metal;


// Structure for constant buffer object (aka "Uniform Buffer")
struct Scene
{
    float4x4        vpMatrix;
    float4x4        vMatrix;
    float4x4        wMatrix;
    packed_float3   lightVec;
    float           texScale;
    float           tessLevelInner;
    float           tessLevelOuter;
    float           maxHeightFactor;
    float           shininessPower;
};

struct InputVS
{
    float3 position     [[attribute(0)]];
    float3 normal       [[attribute(1)]];
    float3 tangent      [[attribute(2)]];
    float3 bitangent    [[attribute(3)]];
    float2 texCoord     [[attribute(4)]];
};

using OutputHS = MTLQuadTessellationFactorsHalf;

struct OutputDS
{
    float4 screenPos [[position]];
    float3 viewPos;
    float3 normal;
    float3 tangent;
    float3 bitangent;
    float2 texCoord;
};


// VERTEX SHADER

// In Metal, LLGL only runs a compute shader to construct the patch constant values.
// The vertex shader as in D3D must be integrated into the post-tessellation vertex shader (aka domain shader).


// HULL SHADER

OutputHS PatchConstantFuncHS(constant Scene& scene)
{
    OutputHS outp;

    outp.edgeTessellationFactor[0] = scene.tessLevelOuter;
    outp.edgeTessellationFactor[1] = scene.tessLevelOuter;
    outp.edgeTessellationFactor[2] = scene.tessLevelOuter;
    outp.edgeTessellationFactor[3] = scene.tessLevelOuter;

    outp.insideTessellationFactor[0] = scene.tessLevelInner;
    outp.insideTessellationFactor[1] = scene.tessLevelInner;

    return outp;
}

kernel void HS(
    constant Scene&     scene       [[buffer(1)]],
    device OutputHS*    patchTess   [[buffer(30)]],
    uint                id          [[thread_position_in_grid]])
{
    patchTess[id] = PatchConstantFuncHS(scene);
}


// DOMAIN SHADER

float InterpolateHeightAlongEdges(float2 coord)
{
    float2 t = 1.0 - abs(coord*2.0 - 1.0);
    return smoothstep(0.0, 0.2, min(t.x, t.y));
}

#define INTERPOLATE_PATCH(COMP)                         \
    mix(                                                \
        mix(patch[0].COMP, patch[1].COMP, tessCoord.x), \
        mix(patch[2].COMP, patch[3].COMP, tessCoord.x), \
        tessCoord.y                                     \
    )

[[patch(quad, 4)]]
vertex OutputDS DS(
    patch_control_point<InputVS>    patch           [[stage_in]],
    constant Scene&                 scene           [[buffer(1)]],
    float2                          tessCoord       [[position_in_patch]],
    sampler                         linearSampler   [[sampler(2)]],
    texture2d<float>                heightMap       [[texture(6)]])
{
    OutputDS outp;

    // Interpolate world position
    float3 interpolatedWorldPos = INTERPOLATE_PATCH(position);
    float2 interpolatedTexCoord = INTERPOLATE_PATCH(texCoord);
    float2 scaledTexCoord = (interpolatedTexCoord - 0.5)*scene.texScale + 0.5;

    // Sample height map and create bump by moving along the patch normal vector
    float heightFactor = scene.maxHeightFactor * InterpolateHeightAlongEdges(interpolatedTexCoord);
    float bumpHeight = heightMap.sample(linearSampler, scaledTexCoord, level(0.0)).r * heightFactor;

    // Transform vertex by the view-projection matrix
    outp.normal     = normalize((scene.wMatrix * float4(patch[0].normal, 0)).xyz);
    outp.tangent    = normalize((scene.wMatrix * float4(patch[0].tangent, 0)).xyz);
    outp.bitangent  = normalize((scene.wMatrix * float4(patch[0].bitangent, 0)).xyz);

    interpolatedWorldPos += outp.normal * bumpHeight;
    float4 worldPos = scene.wMatrix * float4(interpolatedWorldPos, 1);

    outp.screenPos  = scene.vpMatrix * worldPos;
    outp.viewPos    = (scene.vMatrix * worldPos).xyz;
    outp.texCoord   = scaledTexCoord;

    return outp;
}


// PIXEL SHADER

float4 PhongShading(constant Scene& scene, float4 albedo, float3 normal, float shininess, float3 viewVec)
{
    // Diffuse lighting
    float  lambertian   = saturate(dot(normal, scene.lightVec));
    float3 diffuse      = albedo.rgb * lambertian;

    // Specular lighting
    float3 halfVec      = normalize(scene.lightVec - viewVec);
    float specAngle     = saturate(dot(normal, halfVec));
    float3 specular     = (float3)(shininess * pow(specAngle, scene.shininessPower));

    return float4(diffuse + specular, albedo.a);
}

fragment float4 PS(
    OutputDS            inp             [[stage_in]],
    constant Scene&     scene           [[buffer(1)]],
    sampler             linearSampler   [[sampler(2)]],
    texture2d<float>    colorMap        [[texture(3)]],
    texture2d<float>    normalMap       [[texture(4)]],
    texture2d<float>    specularMap     [[texture(5)]])
{
    // Sample color, normal, and specular maps
    float4 albedo = colorMap.sample(linearSampler, inp.texCoord);
    float3 normal = normalize(normalMap.sample(linearSampler, inp.texCoord).rgb * 2.0 - 1.0);
    float roughness = specularMap.sample(linearSampler, inp.texCoord).r;
    float shininess = 1.0 - roughness;

    // Transform normal from world-space into tangent-space
    float3x3 tangentSpace = float3x3(
        normalize(inp.tangent),
        normalize(inp.bitangent),
        normalize(inp.normal)
    );
    normal = tangentSpace * normal;

    // Compute simple phong shading
    return PhongShading(scene, albedo, normal, shininess, normalize(inp.viewPos));
}



