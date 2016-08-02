/*
 * Test2_OpenGL.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/LLGL.h>
#include <memory>
#include <iostream>
#include <string>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#include <gl/GL.h>
#pragma comment(lib, "opengl32.lib")
#endif


int main()
{
    try
    {
        // Load render system module
        auto renderSystem = LLGL::RenderSystem::Load("OpenGL");

        // Create render context
        LLGL::RenderContextDescriptor contextDesc;

        /*contextDesc.profileOpenGL.extProfile    = true;
        contextDesc.profileOpenGL.coreProfile   = true;
        contextDesc.profileOpenGL.version       = LLGL::OpenGLVersion::OpenGL_3_0;*/

        contextDesc.videoMode.screenWidth   = 800;
        contextDesc.videoMode.screenHeight  = 600;

        auto renderContext = renderSystem->CreateRenderContext(contextDesc);

        // Show renderer info
        auto info = renderContext->QueryRendererInfo();

        std::cout << "Renderer:         " << info[LLGL::RendererInfo::Version] << std::endl;
        std::cout << "Vendor:           " << info[LLGL::RendererInfo::Vendor] << std::endl;
        std::cout << "Hardware:         " << info[LLGL::RendererInfo::Hardware] << std::endl;
        std::cout << "Shading Language: " << info[LLGL::RendererInfo::ShadingLanguageVersion] << std::endl;

        // Setup window title
        auto& window = renderContext->GetWindow();

        auto title = "LLGL Test 2 ( " + renderSystem->GetName() + " )";
        window.SetTitle(std::wstring(title.begin(), title.end()));

        // Setup input controller
        auto input = std::make_shared<LLGL::Input>();
        window.AddEventListener(input);

        // Main loop
        while (window.ProcessEvents() && !input->KeyPressed(LLGL::Key::Escape))
        {

            #ifdef _WIN32
            glClearColor(0, 1, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            #endif

            renderContext->Present();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
