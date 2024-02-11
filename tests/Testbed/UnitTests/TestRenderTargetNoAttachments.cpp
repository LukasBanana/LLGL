/*
 * TestRenderTargetNoAttachments.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"


DEF_TEST( RenderTargetNoAttachments )
{
    // Create render target with no attachemnts
    RenderTargetDescriptor target1Desc;
    {
        target1Desc.resolution = Extent2D{ 512, 512 };
    }
    CREATE_RENDER_TARGET(target1, target1Desc, "target1{512x512}");

    // Create render target with no attachemnts and multi-sampling
    RenderTargetDescriptor target2Desc;
    {
        target2Desc.resolution  = Extent2D{ 512, 512 };
        target2Desc.samples     = 8;
    }
    CREATE_RENDER_TARGET(target2, target2Desc, "target2{512x512x8msaa}");

    // Create target using a render pass with no attachments
    RenderPassDescriptor pass3Desc;
    {
        pass3Desc.debugName = "pass3";
    }
    RenderPass* pass3 = renderer->CreateRenderPass(pass3Desc);

    RenderTargetDescriptor target3Desc;
    {
        target3Desc.renderPass  = pass3;
        target3Desc.resolution  = Extent2D{ 800, 600 };
    }
    CREATE_RENDER_TARGET(target3, target3Desc, "target3{800x600}");

    // Create target using a render pass with no attachments
    RenderPassDescriptor pass4Desc;
    {
        pass4Desc.debugName = "pass4{8msaa}";
        pass4Desc.samples   = 8;
    }
    RenderPass* pass4 = renderer->CreateRenderPass(pass4Desc);

    RenderTargetDescriptor target4Desc;
    {
        target4Desc.renderPass  = pass4;
        target4Desc.resolution  = Extent2D{ 800, 600 };
        target4Desc.samples     = pass4Desc.samples;
    }
    CREATE_RENDER_TARGET(target4, target4Desc, "target4{800x600x8msaa}");

    // Delete old render targets
    renderer->Release(*target1);
    renderer->Release(*target2);
    renderer->Release(*target3);
    renderer->Release(*target4);

    return TestResult::Passed;
}

