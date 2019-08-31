/*
 * Test_ShaderReflect.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Utility.h>
#include "Helper.h"

int main()
{
    try
    {
        // Setup profiler and debugger
        auto profiler = std::make_shared<LLGL::RenderingProfiler>();
        auto debugger = std::make_shared<LLGL::RenderingDebugger>();

        // Load render system module
        auto renderer = LLGL::RenderSystem::Load("Vulkan", profiler.get(), debugger.get());

        // Create render context
        LLGL::RenderContextDescriptor contextDesc;
        contextDesc.videoMode.resolution = { 800, 600 };

        auto context = renderer->CreateRenderContext(contextDesc);

        // Create command buffer
        auto commandQueue = renderer->GetCommandQueue();
        auto commands = renderer->CreateCommandBuffer();

        // Load shader
        auto computeShader = renderer->CreateShader(LLGL::ShaderDescFromFile(LLGL::ShaderType::Compute, "Shaders/SpirvReflectTest.comp.spv"));

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

        // Reflect shader
        LLGL::ShaderReflection reflect;
        shaderProgram->Reflect(reflect);

        LLGL::Extent3D workGroupSize;
        shaderProgram->GetWorkGroupSize(workGroupSize);

        std::cout << "Vertex Attributes:" << std::endl;
        for (const LLGL::VertexAttribute& attr : reflect.vertexAttributes)
        {
            std::cout << "  " << attr.name << "@" << attr.offset << std::endl;
        }

        std::cout << "Resources:" << std::endl;
        for (const LLGL::ShaderResource& resc : reflect.resources)
        {
            std::cout << "  " << resc.binding.name << "@" << resc.binding.slot << std::endl;
        }

        for (const LLGL::ShaderUniform& unif : reflect.uniforms)
        {
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
