/*
 * TestbedContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"


#define ENABLE_DEBUGGER 1

TestbedContext::TestbedContext(const char* moduleName)
{
    RenderSystemDescriptor rendererDesc;
    {
        rendererDesc.moduleName = moduleName;
        rendererDesc.flags      = RenderSystemFlags::DebugDevice;
        #if ENABLE_DEBUGGER
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
    RUN_TEST( BufferToTextureCopy       );
    RUN_TEST( TextureCopy               );
    RUN_TEST( TextureToBufferCopy       );
    RUN_TEST( TextureWriteAndRead       );
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

TestResult TestbedContext::CreateBuffer(const BufferDescriptor& desc, const char* name, Buffer** output, const void* initialData)
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
        Log::Errorf("Mismatch between buffer descriptor (size = %" PRIu64 ") and actual buffer (size = %" PRIu64 ")", desc.size, resultDesc.size);
        return TestResult::FailedMismatch;
    }

    // Return buffer to output or delete right away if no longer neeeded
    if (output != nullptr)
        *output = buf;
    else
        renderer->Release(*buf);

    return TestResult::Passed;
}
