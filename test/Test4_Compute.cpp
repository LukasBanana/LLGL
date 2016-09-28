/*
 * Test4_Compute.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Helper.h"

// Fill an array list of 4D-vectors for testing purposes
static std::vector<Gs::Vector4f> GetTestVector(std::size_t size)
{
    std::vector<Gs::Vector4f> vec(size);

    for (unsigned int i = 0; i < size; ++i)
    {
        auto f = static_cast<float>(i + 1);
        vec[i] = Gs::Vector4f(1, f, 1.0f / f, 0.1f * f);
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
        
        // Change window title
        auto title = "LLGL Test 4: Compute ( " + renderer->GetName() + " )";
        context->GetWindow().SetTitle(std::wstring(title.begin(), title.end()));

        // Quit if compute shaders are not supported
        auto renderCaps = renderer->QueryRenderingCaps();
        if (!renderCaps.hasComputeShaders)
            throw std::runtime_error("compute shaders are not supported by renderer");

        // Create storage buffer
        static const unsigned int vecSize = 128;
        auto vec = GetTestVector(vecSize);

        auto storageBuffer = renderer->CreateStorageBuffer(sizeof(Gs::Vector4f)*vecSize, LLGL::BufferUsage::Static, vec.data());

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
        context->SetStorageBuffer(*storageBuffer, 0);
        context->SetComputePipeline(*pipeline);

        // Dispatch compute shader (with 1*1*1 work groups only) and measure elapsed time with timer query
        context->BeginQuery(*timerQuery);
        {
            context->DispatchCompute({ 1, 1, 1 });
        }
        context->EndQuery(*timerQuery);

        // Wait until the GPU has completed all work, to be sure we can evaluate the storage buffer
        context->SyncGPU();

        // Evaluate compute shader
        auto mappedBuffer = context->MapStorageBuffer(*storageBuffer, LLGL::BufferCPUAccess::ReadOnly);
        {
            // Show result
            auto vecBuffer = reinterpret_cast<const Gs::Vector4f*>(mappedBuffer);
            std::cout << "compute shader output: average vector = " << vecBuffer[0] << std::endl;

            // Show elapsed time from timer query
            std::uint64_t result = 0;
            while (!context->QueryResult(*timerQuery, result)) { /* wait until the result is available */ }
            std::cout << "compute shader duration: " << static_cast<double>(result) / 1000000 << " ms" << std::endl;
        }
        context->UnmapStorageBuffer();

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
