/*
 * Test_ShaderReflect.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/LLGL.h>
#include <LLGL/Utils/Utility.h>

int main()
{
    try
    {
        LLGL::Log::RegisterCallbackStd();

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

        LLGL::Log::Printf("Resources:\n");
        for (const LLGL::ShaderResourceReflection& resc : reflect.resources)
        {
            LLGL::Log::Printf("  %s @ %u\n", resc.binding.name.c_str(), resc.binding.slot.index);
        }

        LLGL::Log::Printf("Uniforms:\n");
        for (const LLGL::UniformDescriptor& unif : reflect.uniforms)
        {
            LLGL::Log::Printf("  %s", unif.name.c_str());
            if (unif.arraySize > 0)
                LLGL::Log::Printf("[%u]", unif.arraySize);
            LLGL::Log::Printf("\n");
        }

        LLGL::Log::Printf("Vertex Input Attributes:\n");
        for (const LLGL::VertexAttribute& attr : reflect.vertex.inputAttribs)
        {
            LLGL::Log::Printf("  %s @ %u\n", attr.name.c_str(), attr.location);
        }

        LLGL::Log::Printf("Vertex Output Attributes:\n");
        for (const LLGL::VertexAttribute& attr : reflect.vertex.outputAttribs)
        {
            LLGL::Log::Printf("  %s @ %u\n", attr.name.c_str(), attr.location);
        }

        LLGL::Log::Printf("Fragment Output Attributes:\n");
        for (const LLGL::FragmentAttribute& attr : reflect.fragment.outputAttribs)
        {
            LLGL::Log::Printf("  %s @ %u\n", attr.name.c_str(), attr.location);
        }

        const auto& workGroupSize = reflect.compute.workGroupSize;
        LLGL::Log::Printf("Compute Work Group Size: %u x %u x %u\n", workGroupSize.width, workGroupSize.height, workGroupSize.depth);
    }
    catch (const std::exception& e)
    {
        LLGL::Log::Errorf("%s\n", e.what());
    }

    #ifdef _WIN32
    system("pause");
    #endif

    return 0;
}
