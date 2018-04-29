/*
 * Test4_Compute.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Helper.h"

// Fill an array list of 4D-vectors for testing purposes
static std::vector<Gs::Vector4f> GetTestVector(std::size_t size)
{
    std::vector<Gs::Vector4f> vec(size);

    for (unsigned int i = 0; i < size; ++i)
    {
        auto x = static_cast<float>(i + 1);
        vec[i] = Gs::Vector4f(1, x, 1.0f / x, 0.1f * x);
    }

    return vec;
}

int main()
{
    try
    {
        // Setup profiler and debugger
        auto profiler = std::make_shared<LLGL::RenderingProfiler>();
        auto debugger = std::make_shared<TestDebugger>();

        // Load render system module
        auto renderer = LLGL::RenderSystem::Load("OpenGL", profiler.get(), debugger.get());

        // Create render context
        LLGL::RenderContextDescriptor contextDesc;
        contextDesc.videoMode.resolution = { 800, 600 };

        auto context = renderer->CreateRenderContext(contextDesc);
        
        // Create command buffer
        auto commands = renderer->CreateCommandBufferExt();
        if (!commands)
            throw std::runtime_error("failed to create extended command buffer");

        // Change window title
        auto title = "LLGL Test 4: Compute ( " + renderer->GetName() + " )";

        auto& window = static_cast<LLGL::Window&>(context->GetSurface());
        window.SetTitle(std::wstring(title.begin(), title.end()));

        // Quit if compute shaders are not supported
        const auto& renderCaps = renderer->GetRenderingCaps();
        if (!renderCaps.hasComputeShaders)
            throw std::runtime_error("compute shaders are not supported by renderer");

        // Create storage buffer
        static const unsigned int vecSize = 128;
        auto vec = GetTestVector(vecSize);

        LLGL::BufferDescriptor storageBufferDesc;
        {
            storageBufferDesc.type  = LLGL::BufferType::Storage;
            storageBufferDesc.size  = sizeof(Gs::Vector4f)*vecSize;
            storageBufferDesc.flags = LLGL::BufferFlags::DynamicUsage;
        }
        auto storageBuffer = renderer->CreateBuffer(storageBufferDesc, vec.data());

        // Load shader
        auto computeShader = renderer->CreateShader(LLGL::ShaderType::Compute);

        auto shaderSource = ReadFileContent("ComputeShader.glsl");
        if (!computeShader->Compile(shaderSource))
            std::cerr << computeShader->QueryInfoLog() << std::endl;

        // Create shader program
        auto shaderProgram = renderer->CreateShaderProgram();

        shaderProgram->AttachShader(*computeShader);
        if (!shaderProgram->LinkShaders())
            std::cerr << shaderProgram->QueryInfoLog() << std::endl;

        // Create timer query
        LLGL::QueryDescriptor queryDesc;
        {
            queryDesc.type = LLGL::QueryType::TimeElapsed;
        }
        auto timerQuery = renderer->CreateQuery(queryDesc);

        // Create graphics pipeline
        auto pipeline = renderer->CreateComputePipeline({ shaderProgram });

        // Set resources
        commands->SetStorageBuffer(*storageBuffer, 0);
        commands->SetComputePipeline(*pipeline);

        // Dispatch compute shader (with 1*1*1 work groups only) and measure elapsed time with timer query
        commands->BeginQuery(*timerQuery);
        {
            commands->Dispatch(1, 1, 1);
        }
        commands->EndQuery(*timerQuery);

        // Wait until the GPU has completed all work, to be sure we can evaluate the storage buffer
        renderer->GetCommandQueue()->WaitForFinish();

        // Evaluate compute shader
        auto mappedBuffer = renderer->MapBuffer(*storageBuffer, LLGL::BufferCPUAccess::ReadOnly);
        {
            // Show result
            auto vecBuffer = reinterpret_cast<const Gs::Vector4f*>(mappedBuffer);
            std::cout << "compute shader output: average vector = " << vecBuffer[0] << std::endl;

            // Show elapsed time from timer query
            std::uint64_t result = 0;
            while (!commands->QueryResult(*timerQuery, result)) { /* wait until the result is available */ }
            std::cout << "compute shader duration: " << static_cast<double>(result) / 1000000 << " ms" << std::endl;
        }
        renderer->UnmapBuffer(*storageBuffer);

    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    #ifdef _WIN32
    system("pause");
    #endif

    return 0;
}
