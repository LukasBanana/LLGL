/*
 * TestbedContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/TypeNames.h>
#include <string.h>


#define ENABLE_GPU_DEBUGGER 1
#define ENABLE_CPU_DEBUGGER 0

static bool HasArgument(int argc, char* argv[], const char* search)
{
    for (int i = 0; i < argc; ++i)
    {
        if (::strcmp(argv[i], search) == 0)
            return true;
    }
    return false;
}

TestbedContext::TestbedContext(const char* moduleName, int argc, char* argv[]) :
    showTiming { HasArgument(argc, argv, "-t") || HasArgument(argc, argv, "--timing") },
    fastTest   { HasArgument(argc, argv, "-fast")                                     }
{
    RenderSystemDescriptor rendererDesc;
    {
        rendererDesc.moduleName = moduleName;
        #if ENABLE_GPU_DEBUGGER
        rendererDesc.flags      = RenderSystemFlags::DebugDevice;
        #endif
        #if ENABLE_CPU_DEBUGGER
        rendererDesc.profiler   = &profiler;
        rendererDesc.debugger   = &debugger;
        #endif
    }
    if ((renderer = RenderSystem::Load(rendererDesc)) != nullptr)
    {
        // Create swap chain
        SwapChainDescriptor swapChainDesc;
        {
            swapChainDesc.resolution.width  = 800;
            swapChainDesc.resolution.height = 600;
        }
        swapChain = renderer->CreateSwapChain(swapChainDesc);

        // Show swap-chain surface
        Window& wnd = CastTo<Window>(swapChain->GetSurface());
        wnd.SetTitle("LLGL Testbed - " + std::string(moduleName));
        wnd.Show();

        // Get command queue
        cmdQueue = renderer->GetCommandQueue();

        // Create primary command buffer
        CommandBufferDescriptor cmdBufferDesc;
        {
            cmdBufferDesc.flags = CommandBufferFlags::ImmediateSubmit;
        }
        cmdBuffer = renderer->CreateCommandBuffer(cmdBufferDesc);

        // Query rendering capabilities
        caps = renderer->GetRenderingCaps();
    }
}

void TestbedContext::RunAllTests()
{
    #define RUN_TEST(TEST)                                                          \
        {                                                                           \
            const TestResult result = RunTest(                                      \
                std::bind(&TestbedContext::Test##TEST, this, std::placeholders::_1) \
            );                                                                      \
            EvaluateTestResult(result, #TEST);                                      \
        }

    RUN_TEST( CommandBufferSubmit       );
    RUN_TEST( BufferWriteAndRead        );
    RUN_TEST( BufferMap                 );
    RUN_TEST( BufferFill                );
    RUN_TEST( BufferUpdate              );
    RUN_TEST( BufferCopy                );
    RUN_TEST( TextureTypes              );
    RUN_TEST( TextureWriteAndRead       );
    RUN_TEST( TextureCopy               );
    RUN_TEST( TextureToBufferCopy       );
    RUN_TEST( BufferToTextureCopy       );
    RUN_TEST( DepthBuffer               );
    RUN_TEST( StencilBuffer             );
    RUN_TEST( RenderTargetNoAttachments );
    RUN_TEST( RenderTarget1Attachment   );
    RUN_TEST( RenderTargetNAttachments  );
}

static const char* TestResultToStr(TestResult result)
{
    switch (result)
    {
        case TestResult::Passed:            return "Ok";
        case TestResult::FailedMismatch:    return "FAILED - MISMATCH";
        case TestResult::FailedErrors:      return "FAILED - ERRORS";
        default:                            return "UNDEFINED";
    }
}

void TestbedContext::EvaluateTestResult(TestResult result, const char* name)
{
    Log::Printf("Test %s: [ %s ]\n", name, TestResultToStr(result));
}

TestResult TestbedContext::RunTest(const std::function<TestResult(unsigned)>& callback)
{
    TestResult result = TestResult::Continue;

    for (unsigned frame = 0; swapChain->GetSurface().ProcessEvents() && result == TestResult::Continue; ++frame)
    {
        result = callback(frame);
        swapChain->Present();
    }

    return result;
}

TestResult TestbedContext::CreateBuffer(
    const BufferDescriptor& desc,
    const char*             name,
    Buffer**                output,
    const void*             initialData)
{
    // Create buffer
    Buffer* buf = renderer->CreateBuffer(desc, initialData);

    if (buf == nullptr)
    {
        Log::Errorf("Failed to create buffer: %s\n", name);
        return TestResult::FailedErrors;
    }

    buf->SetName(name);

    // Match buffer descriptor with input
    BufferDescriptor resultDesc = buf->GetDesc();

    if (resultDesc.size < desc.size)
    {
        Log::Errorf(
            "Mismatch between buffer \"%s\" descriptor (size = %" PRIu64 ") and actual buffer (size = %" PRIu64 ")\n",
            name, desc.size, resultDesc.size
        );
        return TestResult::FailedMismatch;
    }

    // Return buffer to output or delete right away if no longer neeeded
    if (output != nullptr)
        *output = buf;
    else
        renderer->Release(*buf);

    return TestResult::Passed;
}

TestResult TestbedContext::CreateTexture(
    const LLGL::TextureDescriptor&  desc,
    const char*                     name,
    LLGL::Texture**                 output,
    const LLGL::SrcImageDescriptor* initialImage)
{
    // Create texture
    Texture* tex = renderer->CreateTexture(desc, initialImage);

    if (tex == nullptr)
    {
        Log::Errorf("Failed to create texture: %s\n", name);
        return TestResult::FailedErrors;
    }

    tex->SetName(name);

    // Match texture descriptor with input
    TextureDescriptor resultDesc = tex->GetDesc();

    if (resultDesc.type != desc.type)
    {
        Log::Errorf(
            "Mismatch between texture \"%s\" descriptor (type = %s) and actual texture (type = %s)\n",
            name, ToString(desc.type), ToString(resultDesc.type)
        );
        return TestResult::FailedMismatch;
    }

    if (resultDesc.extent != desc.extent)
    {
        Log::Errorf(
            "Mismatch between texture \"%s\" descriptor (extent = %u x %u x %u) and actual texture (extent = %u x %u x %u)\n",
            name,
            desc.extent.width, desc.extent.height, desc.extent.depth,
            resultDesc.extent.width, resultDesc.extent.height, resultDesc.extent.depth
        );
        return TestResult::FailedMismatch;
    }

    if (resultDesc.arrayLayers != desc.arrayLayers)
    {
        Log::Errorf(
            "Mismatch between texture \"%s\" descriptor (arrayLayers = %u) and actual texture (arrayLayers = %u)\n",
            name, desc.arrayLayers, resultDesc.arrayLayers
        );
        return TestResult::FailedMismatch;
    }

    const std::uint32_t expectedMipLevels = NumMipLevels(desc);
    if (resultDesc.mipLevels != expectedMipLevels)
    {
        if (expectedMipLevels != desc.mipLevels)
        {
            Log::Errorf(
                "Mismatch between texture \"%s\" descriptor (mipLevels = %u; deduced from %u) and actual texture (mipLevels = %u)\n",
                name, expectedMipLevels, desc.mipLevels, resultDesc.mipLevels
            );
        }
        else
        {
            Log::Errorf(
                "Mismatch between texture \"%s\" descriptor (mipLevels = %u) and actual texture (mipLevels = %u)\n",
                name, expectedMipLevels, resultDesc.mipLevels
            );
        }
        return TestResult::FailedMismatch;
    }

    // Check if actual samples are less; renderer is allowed to have a higher number of samples (OpenGL does that for instance)
    if (resultDesc.samples < desc.samples)
    {
        Log::Errorf(
            "Mismatch between texture \"%s\" descriptor (samples = %u) and actual texture (samples = %u)\n",
            name, desc.samples, resultDesc.samples
        );
        return TestResult::FailedMismatch;
    }

    // Check all MIP-levels
    for (std::uint32_t level = 0; level < resultDesc.mipLevels; ++level)
    {
        const Extent3D resultMipExtent = tex->GetMipExtent(level);
        const Extent3D expectedMipExtent = GetMipExtent(desc, level);
        if (resultMipExtent != expectedMipExtent)
        {
            Log::Errorf(
                "Mismatch between texture \"%s\" MIP [%u] extent (%u x %u x %u) and actual extent (%u x %u x %u)\n",
                name, level,
                resultMipExtent.width, resultMipExtent.height, resultMipExtent.depth,
                expectedMipExtent.width, expectedMipExtent.height, expectedMipExtent.depth
            );
            return TestResult::FailedMismatch;
        }
    }

    // Return texture to output or delete right away if no longer neeeded
    if (output != nullptr)
        *output = tex;
    else
        renderer->Release(*tex);

    return TestResult::Passed;
}

TestResult TestbedContext::CreateRenderTarget(
    const LLGL::RenderTargetDescriptor& desc,
    const char*                         name,
    LLGL::RenderTarget**                output)
{
    // Create render target
    RenderTarget* target = renderer->CreateRenderTarget(desc);

    if (target == nullptr)
    {
        Log::Errorf("Failed to create render target: %s\n", name);
        return TestResult::FailedErrors;
    }

    target->SetName(name);

    // Match render target attributes with input
    const Extent2D resultResolution = target->GetResolution();
    if (resultResolution != desc.resolution)
    {
        Log::Errorf(
            "Mismatch between render target \"%s\" descriptor (resolution = %u x %u) and actual render target (resolution = %u x %u)\n",
            name,
            desc.resolution.width, desc.resolution.height,
            resultResolution.width, resultResolution.height
        );
        return TestResult::FailedMismatch;
    }

    const std::uint32_t resultSamples = target->GetSamples();
    if (resultSamples != desc.samples)
    {
        Log::Errorf(
            "Mismatch between render target \"%s\" descriptor (samples = %u) and actual render target (samples = %u)\n",
            name, desc.samples, resultSamples
        );
        return TestResult::FailedMismatch;
    }

    auto HasAttachment = [](const AttachmentDescriptor& attachment) -> bool
    {
        return (attachment.format != Format::Undefined || attachment.texture != nullptr);
    };

    auto CountAttachments = [&HasAttachment](const AttachmentDescriptor* attachments, std::uint32_t maxCount) -> std::uint32_t
    {
        std::uint32_t n = 0;
        while (n < maxCount && HasAttachment(attachments[n]))
            ++n;
        return n;
    };

    auto AttachmentFormat = [](const AttachmentDescriptor& attachment) -> Format
    {
        if (attachment.format != Format::Undefined)
            return attachment.format;
        if (attachment.texture != nullptr)
            return attachment.texture->GetFormat();
        return Format::Undefined;
    };

    const std::uint32_t expectedAttachments = CountAttachments(desc.colorAttachments, LLGL_MAX_NUM_COLOR_ATTACHMENTS);
    const std::uint32_t resultAttachments = target->GetNumColorAttachments();
    if (resultAttachments != expectedAttachments)
    {
        Log::Errorf(
            "Mismatch between render target \"%s\" descriptor (colorAttachments = %u) and actual render target (colorAttachments = %u)\n",
            name, expectedAttachments, resultAttachments
        );
        return TestResult::FailedMismatch;
    }

    const Format expectedDepthStencilFormat = AttachmentFormat(desc.depthStencilAttachment);
    if (IsDepthFormat(expectedDepthStencilFormat) && !target->HasDepthAttachment())
    {
        Log::Errorf("Mismatch between render target \"%s\" descriptor (with depth attachment) and actual render target (no depth attachment)\n", name);
        return TestResult::FailedMismatch;
    }
    if (IsStencilFormat(expectedDepthStencilFormat) && !target->HasStencilAttachment())
    {
        Log::Errorf("Mismatch between render target \"%s\" descriptor (with stencil attachment) and actual render target (no stencil attachment)\n", name);
        return TestResult::FailedMismatch;
    }

    // Return render-target to output or delete right away if no longer neeeded
    if (output != nullptr)
        *output = target;
    else
        renderer->Release(*target);

    return TestResult::Passed;
}
