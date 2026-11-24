/*
 * TestResolveModes.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"


/*
Alternates disabled and average color resolves on a multisampled render target. Checks that disabled passes leave
the resolve texture unchanged and that subsequent passes load and resolve the stored multisample contents.
Skipped on backends without resolve mode support and devices without multisampling.
*/
DEF_TEST( ResolveModes )
{
    const int rendererID = renderer->GetRendererID();
    if (rendererID != RendererID::Vulkan && rendererID != RendererID::Direct3D12 && rendererID != RendererID::Metal)
        return TestResult::Skipped;

    const std::uint32_t numSamples = std::min<std::uint32_t>(4u, caps.limits.maxColorBufferSamples);
    if (numSamples < 2)
        return TestResult::Skipped;

    const Extent3D extent = { 16, 16, 1 };
    std::vector<std::uint8_t> pixels(extent.width * extent.height * 4, 0);
    for (std::size_t index = 0; index < pixels.size(); index += 4)
    {
        pixels[index + 2] = 255;
        pixels[index + 3] = 255;
    }
    ImageView initialImage;
    initialImage.format   = ImageFormat::RGBA;
    initialImage.dataType = DataType::UInt8;
    initialImage.data     = pixels.data();
    initialImage.dataSize = pixels.size();

    TextureDescriptor resolveDesc;
    resolveDesc.type      = TextureType::Texture2D;
    resolveDesc.format    = Format::RGBA8UNorm;
    resolveDesc.extent    = extent;
    resolveDesc.mipLevels = 1;
    resolveDesc.bindFlags = BindFlags::ColorAttachment | BindFlags::CopySrc;
    CREATE_TEXTURE(resolveTexture, resolveDesc, "resolveMode.resolve", &initialImage);

    TextureDescriptor multisampleDesc = resolveDesc;
    multisampleDesc.type      = TextureType::Texture2DMS;
    multisampleDesc.samples   = numSamples;
    multisampleDesc.bindFlags = BindFlags::ColorAttachment;
    multisampleDesc.miscFlags = MiscFlags::FixedSamples | MiscFlags::NoInitialData;
    CREATE_TEXTURE(multisampleTexture, multisampleDesc, "resolveMode.multisample", nullptr);

    RenderTargetDescriptor targetDesc;
    targetDesc.resolution            = { extent.width, extent.height };
    targetDesc.samples               = numSamples;
    targetDesc.colorAttachments[0]   = multisampleTexture;
    targetDesc.resolveAttachments[0] = resolveTexture;
    CREATE_RENDER_TARGET(target, targetDesc, "resolveMode.target");

    RenderPassDescriptor passDesc;
    passDesc.samples = numSamples;
    passDesc.colorAttachments[0].format      = Format::RGBA8UNorm;
    passDesc.colorAttachments[0].loadOp      = AttachmentLoadOp::Clear;
    passDesc.colorAttachments[0].storeOp     = AttachmentStoreOp::Store;
    passDesc.colorAttachments[0].resolveMode = ResolveMode::Disabled;
    RenderPass* disabledPass = renderer->CreateRenderPass(passDesc);
    passDesc.colorAttachments[0].loadOp      = AttachmentLoadOp::Load;
    passDesc.colorAttachments[0].resolveMode = ResolveMode::Average;
    RenderPass* resolvePass = renderer->CreateRenderPass(passDesc);

    ClearValue red;
    red.color[0] = 1.0f;
    red.color[1] = 0.0f;
    red.color[2] = 0.0f;
    red.color[3] = 1.0f;
    BEGIN();
    {
        cmdBuffer->BeginRenderPass(*target, disabledPass, 1, &red);
        cmdBuffer->Clear(ClearFlags::Color, red);
        /* Restart the Metal render encoder without changing the expected color, testing that the intermediate closure does not resolve. */
        cmdBuffer->Clear(ClearFlags::Color, red);
        cmdBuffer->EndRenderPass();
    }
    END();

    auto CheckColor = [&](std::uint8_t redValue, std::uint8_t blueValue) -> bool
    {
        MutableImageView image;
        image.format   = ImageFormat::RGBA;
        image.dataType = DataType::UInt8;
        image.data     = pixels.data();
        image.dataSize = pixels.size();
        renderer->ReadTexture(*resolveTexture, TextureRegion{ Offset3D{}, extent }, image);
        for (std::size_t index = 0; index < pixels.size(); index += 4)
        {
            if (pixels[index] != redValue || pixels[index + 1] != 0 || pixels[index + 2] != blueValue || pixels[index + 3] != 255)
                return false;
        }
        return true;
    };

    TestResult result = TestResult::Passed;
    if (!CheckColor(0, 255))
    {
        Log::Errorf("ResolveMode: disabled resolve modified the destination\n");
        result = TestResult::FailedMismatch;
    }

    BEGIN();
    {
        cmdBuffer->BeginRenderPass(*target, resolvePass);
        cmdBuffer->EndRenderPass();
    }
    END();
    if (!CheckColor(255, 0))
    {
        Log::Errorf("ResolveMode: subsequent resolve did not preserve the multisample contents\n");
        result = TestResult::FailedMismatch;
    }

    ClearValue blue;
    blue.color[0] = 0.0f;
    blue.color[1] = 0.0f;
    blue.color[2] = 1.0f;
    blue.color[3] = 1.0f;
    BEGIN();
    {
        cmdBuffer->BeginRenderPass(*target, disabledPass, 1, &blue);
        cmdBuffer->Clear(ClearFlags::Color, blue);
        cmdBuffer->EndRenderPass();
    }
    END();
    if (!CheckColor(255, 0))
    {
        Log::Errorf("ResolveMode: switching back to disabled modified the destination\n");
        result = TestResult::FailedMismatch;
    }

    BEGIN();
    {
        cmdBuffer->BeginRenderPass(*target, resolvePass);
        cmdBuffer->EndRenderPass();
    }
    END();
    if (!CheckColor(0, 255))
    {
        Log::Errorf("ResolveMode: switching back to average did not resolve the new contents\n");
        result = TestResult::FailedMismatch;
    }

    renderer->Release(*resolvePass);
    renderer->Release(*disabledPass);
    renderer->Release(*target);
    renderer->Release(*multisampleTexture);
    renderer->Release(*resolveTexture);
    return result;
}
