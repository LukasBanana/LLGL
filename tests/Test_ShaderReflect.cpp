/*
 * Test_ShaderReflect.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/LLGL.h>
#include <LLGL/Utils/Utility.h>
#include <iostream>

int main()
{
    try
    {
        // Setup profiler and debugger
        auto debugger = std::make_shared<LLGL::RenderingDebugger>();

        // Load render system module
        LLGL::RenderSystemDescriptor rendererDesc = "Vulkan";
        {
            rendererDesc.debugger = debugger.get();
        }
        auto renderer = LLGL::RenderSystem::Load(rendererDesc);

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
