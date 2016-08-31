/*
 * Test3_Direct3D12.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/LLGL.h>
#include <Gauss/Gauss.h>


int main()
{
    try
    {
        // Load render system module
        auto renderer = LLGL::RenderSystem::Load("Direct3D12");

        // Create render context
        LLGL::RenderContextDescriptor contextDesc;

        contextDesc.videoMode.resolution    = { 800, 600 };
        //contextDesc.videoMode.fullscreen    = true;

        contextDesc.antiAliasing.enabled    = true;
        contextDesc.antiAliasing.samples    = 8;

        contextDesc.vsync.enabled           = true;

        /*auto context = renderer->CreateRenderContext(contextDesc);
        
        auto& window = context->GetWindow();

        auto renderCaps = context->QueryRenderingCaps();

        auto shadingLang = context->QueryShadingLanguage();*/


    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    system("pause");

    return 0;
}
