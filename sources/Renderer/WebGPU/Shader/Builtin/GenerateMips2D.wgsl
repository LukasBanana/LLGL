/*
 * GenerateMips2D.wgsl
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/*
$INCLUDE_GenerateMips_wgsli  -> Content of file ./GenerateMips.wgsli
$NPOT_TEXTURE_CLASS          -> 0, 1, 2, 3
$SRC_FORMAT                  -> f32, i32, u32
$DST_FORMAT                  -> rgba8unorm, etc.
$PACK_LINEAR_COLOR           -> PackLinearColor, PackLinearColorSRGB
*/

// Tint: `read-write storage textures require the chromium_experimental_read_write_storage_texture extension to be enabled`
enable chromium_experimental_read_write_storage_texture;

$INCLUDE_GenerateMips_wgsli


/* Current MIP-map level configuration */
struct TextureDescriptor
{
    texelSize       : vec2f,    // 1.0 / outMipLevel1.extent
    baseMipLevel    : u32,      // Base MIP-map level of srcMipLevel
    numMipLevels    : u32,      // Number of MIP-map levels to write: [1..4]
    baseArrayLayer  : u32,      // Base array layer of srcMipLevel
}

@group(0) @binding(0)
var srcMipLevel : texture_2d_array<$SRC_FORMAT>; // $SRC_FORMAT expands to scalar format, e.g. f32

/* Next 4 output MIP-map levels and source MIP-map level */
@group(0) @binding(1)
var dstMipLevel1 : texture_storage_2d_array<$DST_FORMAT, read_write>; // $DST_FORMAT expands to vector format, e.g. rgba8unorm

@group(0) @binding(2)
var dstMipLevel2 : texture_storage_2d_array<$DST_FORMAT, read_write>;

@group(0) @binding(3)
var dstMipLevel3 : texture_storage_2d_array<$DST_FORMAT, read_write>;

@group(0) @binding(4)
var dstMipLevel4 : texture_storage_2d_array<$DST_FORMAT, read_write>;

@group(0) @binding(5)
var linearClampSampler : sampler;

@group(0) @binding(6)
var<uniform> texDesc : TextureDescriptor;


/* Primary compute kernel to generate up to 4 MIP-map levels at a time */
@compute @workgroup_size(8, 8, 1)
fn GenerateMips2DCS(
    @builtin(local_invocation_index) groupIndex : u32,
    @builtin(global_invocation_id)   threadID   : vec3<u32>)
{
    let arrayLayer = texDesc.baseArrayLayer + threadID.z;
    var srcColor1 : vec4f = vec4f(0);

    /* Sample source MIP-map level depending on the NPOT texture classification */
    switch ($NPOT_TEXTURE_CLASS)
    {
        default
        {
            let uv1 = texDesc.texelSize * (vec2f(threadID.xy) + vec2f(0.5));
            srcColor1 = textureSampleLevel(srcMipLevel, linearClampSampler, uv1, arrayLayer, f32(texDesc.baseMipLevel));
        }
        case 1
        {
            let uv1 = texDesc.texelSize * (vec2f(threadID.xy) + vec2f(0.25, 0.5));
            let uvOffset = texDesc.texelSize * vec2f(0.5, 0.0);
            srcColor1 = 0.5 * (
                textureSampleLevel(srcMipLevel, linearClampSampler, uv1,            arrayLayer, f32(texDesc.baseMipLevel)) +
                textureSampleLevel(srcMipLevel, linearClampSampler, uv1 + uvOffset, arrayLayer, f32(texDesc.baseMipLevel))
            );
        }
        case 2
        {
            let uv1 = texDesc.texelSize * (vec2f(threadID.xy) + vec2f(0.5, 0.25));
            let uvOffset = texDesc.texelSize * vec2f(0.0, 0.5);
            srcColor1 = 0.5 * (
                textureSampleLevel(srcMipLevel, linearClampSampler, uv1,            arrayLayer, f32(texDesc.baseMipLevel)) +
                textureSampleLevel(srcMipLevel, linearClampSampler, uv1 + uvOffset, arrayLayer, f32(texDesc.baseMipLevel))
            );
        }
        case 3
        {
            let uv1 = texDesc.texelSize * (vec2f(threadID.xy) + vec2f(0.25));
            let uvOffset = texDesc.texelSize * 0.5;
            srcColor1 = 0.25 * (
                textureSampleLevel(srcMipLevel, linearClampSampler, uv1,                          arrayLayer, f32(texDesc.baseMipLevel)) +
                textureSampleLevel(srcMipLevel, linearClampSampler, uv1 + vec2f(uvOffset.x, 0.0), arrayLayer, f32(texDesc.baseMipLevel)) +
                textureSampleLevel(srcMipLevel, linearClampSampler, uv1 + vec2f(0.0, uvOffset.y), arrayLayer, f32(texDesc.baseMipLevel)) +
                textureSampleLevel(srcMipLevel, linearClampSampler, uv1 + uvOffset,               arrayLayer, f32(texDesc.baseMipLevel))
            );
        }
    }

    /* Write 1st output MIP-map level */
    textureStore(dstMipLevel1, threadID.xy, i32(arrayLayer), $PACK_LINEAR_COLOR(srcColor1));

    if (texDesc.numMipLevels == 1)
    {
        return;
    }

    /* Write 2nd output MIP-map level */
    StoreColor(groupIndex, srcColor1);
    workgroupBarrier();

    if ((groupIndex & 0x09) == 0) // BITMASK_XY_EVEN
    {
        let srcColor2 = LoadColor(groupIndex + 0x01u);
        let srcColor3 = LoadColor(groupIndex + 0x08u);
        let srcColor4 = LoadColor(groupIndex + 0x09u);
        srcColor1 = 0.25 * (srcColor1 + srcColor2 + srcColor3 + srcColor4);

        textureStore(dstMipLevel2, threadID.xy / 2, i32(arrayLayer), $PACK_LINEAR_COLOR(srcColor1));
        StoreColor(groupIndex, srcColor1);
    }

    if (texDesc.numMipLevels == 2)
    {
        return;
    }

    /* Write 3rd output MIP-map level */
    workgroupBarrier();

    if ((groupIndex & 0x1B) == 0) // BITMASK_XY_MULTIPLE_OF_4
    {
        let srcColor2 = LoadColor(groupIndex + 0x02u);
        let srcColor3 = LoadColor(groupIndex + 0x10u);
        let srcColor4 = LoadColor(groupIndex + 0x12u);
        srcColor1 = 0.25 * (srcColor1 + srcColor2 + srcColor3 + srcColor4);

        textureStore(dstMipLevel3, threadID.xy / 4, i32(arrayLayer), $PACK_LINEAR_COLOR(srcColor1));
        StoreColor(groupIndex, srcColor1);
    }

    if (texDesc.numMipLevels == 3)
    {
        return;
    }

    /* Write 4th output MIP-map level */
    workgroupBarrier();

    if (groupIndex == 0)
    {
        let srcColor2 = LoadColor(0x04u);
        let srcColor3 = LoadColor(0x20u);
        let srcColor4 = LoadColor(0x24u);
        srcColor1 = 0.25 * (srcColor1 + srcColor2 + srcColor3 + srcColor4);

        textureStore(dstMipLevel4, threadID.xy / 8, i32(arrayLayer), $PACK_LINEAR_COLOR(srcColor1));
    }
}

