/*
 * main.cpp (Tutorial08_Compute)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../tutorial.h"


int main(int argc, char* argv[])
{
    try
    {
        // Load render system module
        auto renderer = LLGL::RenderSystem::Load(GetSelectedRendererModule(argc, argv));

        // Create render context but do not show its window
        LLGL::RenderContextDescriptor contextDesc;
        {
            contextDesc.videoMode.resolution = { 640, 480 };
        }
        auto context = renderer->CreateRenderContext(contextDesc);

        // Create command buffer
        union
        {
            LLGL::CommandBuffer*    commands    = nullptr;
            LLGL::CommandBufferExt* commandsExt;
        };

        commandsExt = renderer->CreateCommandBufferExt();
        if (!commands)
            commands = renderer->CreateCommandBuffer();

        // Initialize buffer data (16 byte pack alignment)
        struct DataBlock
        {
            Gs::Vector4f        position;
            LLGL::ColorRGBAf    color;
        };

        std::vector<DataBlock> inputData, outputData;

        for (int i = 0; i < 10; ++i)
        {
            auto x = static_cast<float>(i + 1);
            inputData.push_back(
                {
                    Gs::Vector4f(x, 1.0f / x, x*x, 1.0f),
                    LLGL::ColorRGBAf(x, x*2.0f, std::sqrt(x), 1.0f),
                }
            );
        }

        outputData.resize(inputData.size());

        // Create storage buffer for input
        LLGL::BufferDescriptor storageBufferDesc;
        {
            storageBufferDesc.type                      = LLGL::BufferType::Storage;
            storageBufferDesc.size                      = static_cast<std::uint32_t>(inputData.size() * sizeof(DataBlock));
            storageBufferDesc.flags                     = LLGL::BufferFlags::DynamicUsage | LLGL::BufferFlags::MapReadAccess;
            storageBufferDesc.storageBuffer.storageType = LLGL::StorageBufferType::RWStructuredBuffer;
            storageBufferDesc.storageBuffer.stride      = sizeof(DataBlock);
        }
        auto storageBuffer = renderer->CreateBuffer(storageBufferDesc, inputData.data());

        // Create shaders
        auto computeShader = renderer->CreateShader(LLGL::ShaderType::Compute);

        // Load compute shader code from file
        auto PrintShaderLog = [&](LLGL::Shader* shader)
        {
            // Print info log (warnings and errors)
            std::string log = shader->QueryInfoLog();
            if (!log.empty())
                std::cerr << log << std::endl;
        };

        auto CompileShader = [&](LLGL::Shader* shader, const std::string& source, const LLGL::ShaderDescriptor& shaderDesc = {})
        {
            // Compile shader
            shader->Compile(source, shaderDesc);
            PrintShaderLog(shader);
        };

        auto LoadBinaryShader = [&](LLGL::Shader* shader, std::vector<char>&& binaryCode, const LLGL::ShaderDescriptor& shaderDesc = {})
        {
            // Load binary into shader
            shader->LoadBinary(std::move(binaryCode), shaderDesc);
            PrintShaderLog(shader);
        };

        const auto& languages = renderer->GetRenderingCaps().shadingLanguages;
        if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::HLSL) != languages.end())
            CompileShader(computeShader, ReadFileContent("shader.hlsl"), { "CS", "cs_5_0" });
        else if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::GLSL) != languages.end())
            CompileShader(computeShader, ReadFileContent("compute.glsl"));
        else if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::SPIRV) != languages.end())
            LoadBinaryShader(computeShader, ReadFileBuffer("compute.spv"));

        // Create shader program which is used as composite
        auto shaderProgram = renderer->CreateShaderProgram();

        // Attach compute shader to the shader program
        shaderProgram->AttachShader(*computeShader);

        // Link shader program and check for errors
        if (!shaderProgram->LinkShaders())
            throw std::runtime_error(shaderProgram->QueryInfoLog());

        // Create pipeline layout for Vulkan and Direct3D 12 render systems
        LLGL::PipelineLayoutDescriptor pipelineLayoutDesc;
        {
            pipelineLayoutDesc.bindings =
            {
                LLGL::LayoutBindingDescriptor { LLGL::ResourceType::StorageBuffer, LLGL::ShaderStageFlags::ComputeStage, 0 }
            };
        }
        auto pipelineLayout = renderer->CreatePipelineLayout(pipelineLayoutDesc);

        // Create resource view heap
        LLGL::ResourceViewHeapDescriptor viewHeapDesc;
        {
            viewHeapDesc.pipelineLayout = pipelineLayout;
            viewHeapDesc.resourceViews  = { LLGL::ResourceViewDesc(storageBuffer) };
        }
        auto resourceViewHeap = renderer->CreateResourceViewHeap(viewHeapDesc);

        // Create compute pipeline
        auto pipeline = renderer->CreateComputePipeline({ shaderProgram, pipelineLayout });

        // Set compute pipeline
        commands->SetComputePipeline(*pipeline);

        // Set storage buffer
        if (resourceViewHeap)
            commands->SetComputeResourceViewHeap(*resourceViewHeap, 0);
        else if (commandsExt)
            commandsExt->SetStorageBuffer(*storageBuffer, 0, LLGL::ShaderStageFlags::ComputeStage);

        // Dispatch compute shader
        commands->Dispatch(static_cast<std::uint32_t>(inputData.size()), 1, 1);

        // Read result
        renderer->GetCommandQueue()->WaitIdle();

        if (auto outputBuffer = renderer->MapBuffer(*storageBuffer, LLGL::BufferCPUAccess::ReadOnly))
        {
            ::memcpy(outputData.data(), outputBuffer, sizeof(DataBlock) * outputData.size());
            renderer->UnmapBuffer(*storageBuffer);
        }

        // Show input and output
        std::cout << "input/output data:" << std::endl;

        for (std::size_t i = 0, n = inputData.size(); i < n; ++i)
        {
            std::cout << "  in.position  = " << inputData[i].position << std::endl;
            std::cout << "  out.position = " << outputData[i].position << std::endl;
            std::cout << std::endl;
        }
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
