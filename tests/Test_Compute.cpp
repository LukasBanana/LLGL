/*
 * Test_Compute.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/LLGL.h>
#include <Gauss/Gauss.h>
#include <vector>

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
        auto debugger = std::make_shared<LLGL::RenderingDebugger>();

        // Load render system module
        auto renderer = LLGL::RenderSystem::Load("OpenGL", profiler.get(), debugger.get());

        // Create render context
        LLGL::RenderContextDescriptor contextDesc;
        contextDesc.videoMode.resolution = { 800, 600 };

        /*auto context = */renderer->CreateRenderContext(contextDesc);

        // Create command buffer
        auto commandQueue = renderer->GetCommandQueue();
        auto commands = renderer->CreateCommandBuffer();

        // Quit if compute shaders are not supported
        const auto& renderCaps = renderer->GetRenderingCaps();
        if (!renderCaps.features.hasComputeShaders)
            throw std::runtime_error("compute shaders are not supported by renderer");

        // Create storage buffer
        static const unsigned int vecSize = 128;
        auto vec = GetTestVector(vecSize);

        LLGL::BufferDescriptor storageBufferDesc;
        {
            storageBufferDesc.size              = sizeof(Gs::Vector4f)*vecSize;
            storageBufferDesc.bindFlags         = LLGL::BindFlags::Storage;
            storageBufferDesc.cpuAccessFlags    = LLGL::CPUAccessFlags::Read;
            storageBufferDesc.miscFlags         = LLGL::MiscFlags::DynamicUsage;
        }
        auto storageBuffer = renderer->CreateBuffer(storageBufferDesc, vec.data());

        // Load shader
        auto computeShader = renderer->CreateShader({ LLGL::ShaderType::Compute, "Shaders/ComputeShader.glsl" });

        if (computeShader->HasErrors())
            std::cerr << computeShader->GetReport() << std::endl;

        // Create shader program
        LLGL::ShaderProgramDescriptor shaderProgramDesc;
        {
            shaderProgramDesc.computeShader = computeShader;
        }
        auto shaderProgram = renderer->CreateShaderProgram(shaderProgramDesc);

        if (shaderProgram->HasErrors())
            std::cerr << shaderProgram->GetReport() << std::endl;

        // Create timer query
        LLGL::QueryHeapDescriptor queryDesc;
        {
            queryDesc.type = LLGL::QueryType::TimeElapsed;
        }
        auto timerQuery = renderer->CreateQueryHeap(queryDesc);

        // Create compute pipeline
        LLGL::ComputePipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram = shaderProgram;
        }
        auto pipeline = renderer->CreatePipelineState(pipelineDesc);

        // Set resources
        commands->Begin();
        {
            commands->SetResource(*storageBuffer, 0, LLGL::BindFlags::Storage, LLGL::StageFlags::ComputeStage);
            commands->SetPipelineState(*pipeline);

            // Dispatch compute shader (with 1*1*1 work groups only) and measure elapsed time with timer query
            commands->BeginQuery(*timerQuery);
            {
                commands->Dispatch(1, 1, 1);
            }
            commands->EndQuery(*timerQuery);
        }
        commands->End();
        commandQueue->Submit(*commands);

        // Show elapsed time from timer query
        std::uint64_t result = 0;
        while (!commandQueue->QueryResult(*timerQuery, 0, 1, &result, sizeof(result)))
        {
            /* wait until the result is available */
        }
        std::cout << "compute shader duration: " << static_cast<double>(result) / 1000000 << " ms" << std::endl;

        // Wait until the GPU has completed all work, to be sure we can evaluate the storage buffer
        renderer->GetCommandQueue()->WaitIdle();

        // Evaluate compute shader
        if (auto mappedBuffer = renderer->MapBuffer(*storageBuffer, LLGL::CPUAccess::ReadOnly))
        {
            // Show result
            auto vecBuffer = reinterpret_cast<const Gs::Vector4f*>(mappedBuffer);
            std::cout << "compute shader output: average vector = " << vecBuffer[0] << std::endl;
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
