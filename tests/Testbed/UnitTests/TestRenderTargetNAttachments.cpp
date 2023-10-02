/*
 * TestRenderTargetNAttachments.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"


DEF_TEST( RenderTargetNAttachments )
{
    ////////////// SINGLE SAMPLING //////////////

    // Create render target with 1 attachemnt
    RenderTargetDescriptor target1Desc;
    {
        target1Desc.resolution          = Extent2D{ 512, 512 };
        target1Desc.colorAttachments[0] = Format::RGBA8UNorm;
        target1Desc.colorAttachments[1] = Format::RG11B10Float;
        target1Desc.colorAttachments[2] = Format::BGRA8UNorm_sRGB;
    }
    CREATE_RENDER_TARGET(target1, target1Desc, "target1{rgba8,rg11b10f,bgra_sRGB}");

    // Create render target with 1 color and depth-stencil attachment
    RenderTargetDescriptor target2Desc;
    {
        target2Desc.resolution              = Extent2D{ 512, 512 };
        target2Desc.colorAttachments[0]     = Format::RGBA8UInt;
        target2Desc.colorAttachments[1]     = Format::R16Float;
        target2Desc.colorAttachments[2]     = Format::RG32SInt;
        target2Desc.depthStencilAttachment  = Format::D24UNormS8UInt;
    }
    CREATE_RENDER_TARGET(target2, target2Desc, "target2{rgba8ui,r16f,rg32s,d24s8}");

    // Create render target with 1 custom color attachment
    TextureDescriptor depthTex1Desc;
    {
        depthTex1Desc.type          = TextureType::Texture2DArray;
        depthTex1Desc.extent.width  = 800;
        depthTex1Desc.extent.height = 600;
        depthTex1Desc.arrayLayers   = 8;
        depthTex1Desc.mipLevels     = 2;
        depthTex1Desc.format        = Format::RGBA8UNorm;
        depthTex1Desc.bindFlags     = BindFlags::ColorAttachment;
    }
    CREATE_TEXTURE(colorTex1, depthTex1Desc, "colorTex1{rgba8[8]}", nullptr);

    RenderTargetDescriptor target3Desc;
    {
        target3Desc.resolution.width    = depthTex1Desc.extent.width;
        target3Desc.resolution.height   = depthTex1Desc.extent.height;
        for (std::uint32_t attachment = 0; attachment < depthTex1Desc.arrayLayers; ++attachment)
        {
            target3Desc.colorAttachments[attachment].texture    = colorTex1;
            target3Desc.colorAttachments[attachment].arrayLayer = attachment;
        }
    }
    CREATE_RENDER_TARGET(target3, target3Desc, "target3{colorTex1[0..7].mip0}");

    RenderTargetDescriptor target4Desc;
    {
        target4Desc.resolution.width    = depthTex1Desc.extent.width / 2;
        target4Desc.resolution.height   = depthTex1Desc.extent.height / 2;
        for (std::uint32_t attachment = 0; attachment < depthTex1Desc.arrayLayers; ++attachment)
        {
            target4Desc.colorAttachments[attachment].texture    = colorTex1;
            target4Desc.colorAttachments[attachment].arrayLayer = attachment;
            target4Desc.colorAttachments[attachment].mipLevel   = 1;
        }
    }
    CREATE_RENDER_TARGET(target4, target4Desc, "target4{colorTex1[0..7].mip1}");

    ////////////// MULTI SAMPLING //////////////

    // Create render target with 1 depth-stencil attachment and multi-sampling
    RenderTargetDescriptor targetMS1Desc;
    {
        targetMS1Desc.resolution                = Extent2D{ 512, 512 };
        targetMS1Desc.colorAttachments[0]       = Format::RGBA8UNorm;
        targetMS1Desc.colorAttachments[1]       = Format::BGRA8UNorm;
        targetMS1Desc.colorAttachments[2]       = Format::RG16Float;
        targetMS1Desc.resolveAttachments[1]     = Format::BGRA8UNorm;
        targetMS1Desc.depthStencilAttachment    = Format::D24UNormS8UInt;
        targetMS1Desc.samples                   = 8;
    }
    CREATE_RENDER_TARGET(targetMS1, targetMS1Desc, "targetMS1{rgba8,bgra8,rg16f,8msaa}");

    // Delete old resources
    renderer->Release(*target1);
    renderer->Release(*target2);
    renderer->Release(*target3);
    renderer->Release(*target4);
    renderer->Release(*targetMS1);
    renderer->Release(*colorTex1);

    return TestResult::Passed;
}

