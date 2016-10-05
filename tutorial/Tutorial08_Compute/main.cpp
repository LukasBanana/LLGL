/*
 * main.cpp (Tutorial08_Compute)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../tutorial.h"


int main(int argc, char* argv[])
{
    try
    {
        // Load render system module
        auto renderer = LLGL::RenderSystem::Load(GetSelectedRendererModule(argc, argv));

        std::cout << "LLGL Renderer: " << renderer->GetName() << std::endl;

        // Create render context but do not show its window
        LLGL::RenderContextDescriptor contextDesc;
        {
            contextDesc.videoMode.resolution = { 640, 480 };
        }
        auto context = renderer->CreateRenderContext(contextDesc);

        // Initialize buffer data
        struct InputData
        {
            Gs::Vector3f    position;
            LLGL::ColorRGBf color;
        };

        std::vector<InputData> inputData;

        // Create storage buffer for input
        LLGL::BufferDescriptor inputBufferDesc;
        {
            inputBufferDesc.type                        = LLGL::BufferType::Storage;
            inputBufferDesc.size                        = inputData.size() * sizeof(InputData);
            inputBufferDesc.usage                       = LLGL::BufferUsage::Static;
            inputBufferDesc.storageBuffer.storageType   = LLGL::StorageBufferType::Generic;
        }
        auto inputBuffer = renderer->CreateBuffer(inputBufferDesc, inputData.data());

        // Create shaders
        auto computeShader = renderer->CreateShader(LLGL::ShaderType::Compute);

        // Load compute shader code from file
        auto CompileShader = [](LLGL::Shader* shader, const LLGL::ShaderSource& code)
        {
            // Compile shader
            shader->Compile(code);

            // Print info log (warnings and errors)
            std::string log = shader->QueryInfoLog();
            if (!log.empty())
                std::cerr << log << std::endl;
        };

        if (renderer->GetRenderingCaps().shadingLanguage >= LLGL::ShadingLanguage::HLSL_2_0)
            CompileShader(computeShader, LLGL::ShaderSource(ReadFileContent("shader.hlsl"), "CS", "cs_5_0"));
        else
            CompileShader(computeShader, ReadFileContent("compute.glsl"));

        // Create shader program which is used as composite
        auto shaderProgram = renderer->CreateShaderProgram();

        // Attach compute shader to the shader program
        shaderProgram->AttachShader(*computeShader);

        // Link shader program and check for errors
        if (!shaderProgram->LinkShaders())
            throw std::runtime_error(shaderProgram->QueryInfoLog());

        // Create compute pipeline
        auto pipeline = renderer->CreateComputePipeline(shaderProgram);

        //to be continued ...


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
