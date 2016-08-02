/*
 * Test2_OpenGL.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/LLGL.h>
#include <Gauss/Gauss.h>
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
        auto renderer = LLGL::RenderSystem::Load("OpenGL");

        // Create render context
        LLGL::RenderContextDescriptor contextDesc;

        contextDesc.videoMode.resolution    = { 800, 600 };
        //contextDesc.videoMode.fullscreen    = true;

        contextDesc.antiAliasing.enabled    = true;
        contextDesc.antiAliasing.samples    = 8;

        contextDesc.vsync.enabled           = true;

        auto context = renderer->CreateRenderContext(contextDesc);

        context->SetClearColor(LLGL::ColorRGBAf(0.3f, 0.3f, 1));

        // Show renderer info
        auto info = context->QueryRendererInfo();

        std::cout << "Renderer:         " << info[LLGL::RendererInfo::Version] << std::endl;
        std::cout << "Vendor:           " << info[LLGL::RendererInfo::Vendor] << std::endl;
        std::cout << "Hardware:         " << info[LLGL::RendererInfo::Hardware] << std::endl;
        std::cout << "Shading Language: " << info[LLGL::RendererInfo::ShadingLanguageVersion] << std::endl;

        // Setup window title
        auto& window = context->GetWindow();

        auto title = "LLGL Test 2 ( " + renderer->GetName() + " )";
        window.SetTitle(std::wstring(title.begin(), title.end()));

        // Setup input controller
        auto input = std::make_shared<LLGL::Input>();
        window.AddEventListener(input);

        // Main loop
        while (window.ProcessEvents() && !input->KeyPressed(LLGL::Key::Escape))
        {
            context->ClearBuffers(LLGL::ClearBuffersFlags::Color);

            #ifdef _WIN32
            
            auto proj = Gs::ProjectionMatrix4f::Planar(
                static_cast<Gs::Real>(contextDesc.videoMode.resolution.x),
                static_cast<Gs::Real>(contextDesc.videoMode.resolution.y)
            );

            glMatrixMode(GL_PROJECTION);
            glLoadMatrixf(proj.Ptr());

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glBegin(GL_LINES);
            {
                glVertex2i(200, 100);
                glVertex2i(400, 200);
            }
            glEnd();

            #endif

            context->Present();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
