/*
 * Test4_Compute.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Helper.h"

int main()
{
    try
    {
        // Load render system module
        auto renderer = LLGL::RenderSystem::Load("OpenGL");

        // Create render context
        LLGL::RenderContextDescriptor contextDesc;
        contextDesc.videoMode.resolution = { 800, 600 };

        auto context = renderer->CreateRenderContext(contextDesc);
        
        auto window = &(context->GetWindow());

        auto title = "LLGL Test 4: Compute ( " + renderer->GetName() + " )";
        window->SetTitle(std::wstring(title.begin(), title.end()));

        auto renderCaps = renderer->QueryRenderingCaps();

        if (!renderCaps.hasComputeShaders)
            throw std::runtime_error("compute shaders are not supported by renderer");

        // Create compute buffer
        static const unsigned int vecSize = 128;
        std::vector<Gs::Vector4f> vec(vecSize);

        for (unsigned int i = 0; i < vecSize; ++i)
        {
            auto f = static_cast<float>(i + 1);
            vec[i] = Gs::Vector4f(1, f, 1.0f / f, 0.1f * f);
        }

        auto computeBuffer = renderer->CreateStorageBuffer();
        renderer->SetupStorageBuffer(*computeBuffer, vec.data(), sizeof(Gs::Vector4f)*vecSize, LLGL::BufferUsage::Static);

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

        auto constBufferDescs = shaderProgram->QueryConstantBuffers();
        auto storeBufferDescs = shaderProgram->QueryStorageBuffers();

        // Create graphics pipeline
        LLGL::ComputePipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram = shaderProgram;
        }
        auto pipeline = renderer->CreateComputePipeline(pipelineDesc);

        // Bind resources
        context->BindStorageBuffer(0, *computeBuffer);
        context->BindComputePipeline(*pipeline);

        // Dispatch compute shader
        context->DispatchCompute({ 1, 1, 1 });

        // Evaluate compute shader
        context->SyncGPU();

        auto mappedBuffer = context->MapStorageBuffer(*computeBuffer, LLGL::BufferCPUAccess::ReadOnly);
        {
            auto vecBuffer = reinterpret_cast<const Gs::Vector4f*>(mappedBuffer);
            std::cout << "compute shader output: average vector = " << vecBuffer[0] << std::endl;
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
