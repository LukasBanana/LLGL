/*
 * TestRenderTarget1Attachment.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"


DEF_TEST( RenderTarget1Attachment )
{
    ////////////// SINGLE SAMPLING //////////////

    // Create render target with 1 attachemnt
    RenderTargetDescriptor target1Desc;
    {
        target1Desc.resolution          = Extent2D{ 512, 512 };
        target1Desc.colorAttachments[0] = Format::RGBA8UNorm;
    }
    CREATE_RENDER_TARGET(target1, target1Desc, "target1{rgba8}");

    // Create render target with 1 color and depth-stencil attachment
    RenderTargetDescriptor target2Desc;
    {
        target2Desc.resolution              = Extent2D{ 512, 512 };
        target2Desc.colorAttachments[0]     = Format::RGBA8UInt;
        target2Desc.depthStencilAttachment  = Format::D24UNormS8UInt;
    }
    CREATE_RENDER_TARGET(target2, target2Desc, "target2{rgba8ui,d24s8}");

    // Create render target with 1 depth-stencil attachment
    RenderTargetDescriptor target3Desc;
    {
        target3Desc.resolution              = Extent2D{ 512, 512 };
        target3Desc.depthStencilAttachment  = Format::D24UNormS8UInt;
    }
    CREATE_RENDER_TARGET(target3, target3Desc, "target3{d24s8}");

    // Create render target with 1 custom depth-stencil attachment
    TextureDescriptor depthTex1Desc;
    {
        depthTex1Desc.extent.width  = 512;
        depthTex1Desc.extent.height = 512;
        depthTex1Desc.format        = Format::D24UNormS8UInt;
        depthTex1Desc.bindFlags     = BindFlags::DepthStencilAttachment;
    }
    CREATE_TEXTURE(depthTex1, depthTex1Desc, "depthTex1{d24s8}", nullptr);

    RenderTargetDescriptor target4Desc;
    {
        target4Desc.resolution              = Extent2D{ 512, 512 };
        target4Desc.depthStencilAttachment  = depthTex1;
    }
    CREATE_RENDER_TARGET(target4, target4Desc, "target4{d24s8-tex}");

    ////////////// MULTI SAMPLING //////////////

    // Create render target with 1 depth-stencil attachment and multi-sampling
    RenderTargetDescriptor targetMS1Desc;
    {
        targetMS1Desc.resolution                = Extent2D{ 512, 512 };
        targetMS1Desc.depthStencilAttachment    = Format::D24UNormS8UInt;
        targetMS1Desc.samples                   = 8;
    }
    CREATE_RENDER_TARGET(targetMS1, targetMS1Desc, "targetMS1{d24s8,8msaa}");

    // Create render target with 1 attachemnt and multi-sampling
    RenderTargetDescriptor targetMS2Desc;
    {
        targetMS2Desc.resolution    = Extent2D{ 512, 512 };
        targetMS2Desc.samples       = 8;
    }
    CREATE_RENDER_TARGET(targetMS2, targetMS2Desc, "targetMS2{512x512x8msaa[1]}");

    #if 0 //TODO

    ////////////// CUSTOM RENDER PASSES //////////////

    // Create target using a render pass with 1 attachment
    RenderPassDescriptor pass3Desc;
    RenderPass* pass3 = renderer->CreateRenderPass(pass3Desc);
    pass3->SetName("pass3");

    RenderTargetDescriptor target3Desc;
    {
        target3Desc.renderPass  = pass3;
        target3Desc.resolution  = Extent2D{ 800, 600 };
    }
    CREATE_RENDER_TARGET(target3, target3Desc, "target3{800x600[1]}");

    // Create target using a render pass with 1 attachment
    RenderPassDescriptor pass4Desc;
    {
        pass4Desc.samples = 8;
    }
    RenderPass* pass4 = renderer->CreateRenderPass(pass4Desc);
    pass4->SetName("pass4{8msaa}");

    RenderTargetDescriptor target4Desc;
    {
        target4Desc.renderPass  = pass4;
        target4Desc.resolution  = Extent2D{ 800, 600 };
        target4Desc.samples     = pass4Desc.samples;
    }
    CREATE_RENDER_TARGET(target4, target4Desc, "target4{800x600x8msaa[1]}");
    #endif

    // Delete old render targets
    renderer->Release(*target2);
    renderer->Release(*target3);
    renderer->Release(*target4);
    renderer->Release(*targetMS1);
    renderer->Release(*targetMS2);
    renderer->Release(*depthTex1);

    return TestResult::Passed;
}

