/*
 * Test_ShaderReflect.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/LLGL.h>
#include <LLGL/Misc/Utility.h>
#include <iostream>

int main()
{
    try
    {
        // Setup profiler and debugger
        auto profiler = std::make_shared<LLGL::RenderingProfiler>();
        auto debugger = std::make_shared<LLGL::RenderingDebugger>();

        // Load render system module
        auto renderer = LLGL::RenderSystem::Load("Vulkan", profiler.get(), debugger.get());

        // Create swap-chain
        LLGL::SwapChainDescriptor swapChainDesc;
        swapChainDesc.resolution = { 800, 600 };

        auto swapChain = renderer->CreateSwapChain(swapChainDesc);

        // Create command buffer
        auto commandQueue = renderer->GetCommandQueue();
        auto commands = renderer->CreateCommandBuffer();

        // Load shader
        auto computeShader = renderer->CreateShader(LLGL::ShaderDescFromFile(LLGL::ShaderType::Compute, "Shaders/SpirvReflectTest.comp.spv"));

        // Reflect shader
        LLGL::ShaderReflection reflect;
        computeShader->Reflect(reflect);

        std::cout << "Resources:" << std::endl;
        for (const LLGL::ShaderResourceReflection& resc : reflect.resources)
        {
            std::cout << "  " << resc.binding.name << " @ " << resc.binding.slot.index << std::endl;
        }

        std::cout << "Uniforms:" << std::endl;
        for (const LLGL::UniformDescriptor& unif : reflect.uniforms)
        {
            std::cout << "  " << unif.name;
            if (unif.arraySize > 0)
                std::cout << '[' << unif.arraySize << ']';
            std::cout << std::endl;
        }

        std::cout << "Vertex Input Attributes:" << std::endl;
        for (const LLGL::VertexAttribute& attr : reflect.vertex.inputAttribs)
        {
            std::cout << "  " << attr.name << " @ " << attr.location << std::endl;
        }

        std::cout << "Vertex Output Attributes:" << std::endl;
        for (const LLGL::VertexAttribute& attr : reflect.vertex.outputAttribs)
        {
            std::cout << "  " << attr.name << " @ " << attr.location << std::endl;
        }

        std::cout << "Fragment Output Attributes:" << std::endl;
        for (const LLGL::FragmentAttribute& attr : reflect.fragment.outputAttribs)
        {
            std::cout << "  " << attr.name << " @ " << attr.location << std::endl;
        }

        const auto& workGroupSize = reflect.compute.workGroupSize;
        std::cout << "Compute Work Group Size: " << workGroupSize.width << " x " << workGroupSize.height << " x " << workGroupSize.depth << std::endl;
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
