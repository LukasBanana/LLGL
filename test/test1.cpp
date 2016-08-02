/*
 * test1.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/LLGL.h>
#include <memory>
#include <iostream>
#include <string>
#include <sstream>


int main()
{
    try
    {
        // Create window
        LLGL::WindowDescriptor windowDesc;

        windowDesc.title        = L"LLGL Test 1";
        windowDesc.visible      = true;
        windowDesc.centered     = true;
        windowDesc.resizable    = true;
        windowDesc.size         = { 640, 480 };

        auto window = LLGL::Window::Create(windowDesc);

        auto input = std::make_shared<LLGL::Input>();
        window->AddEventListener(input);

        auto timer = LLGL::Timer::Create();

        auto pos = window->GetPosition();

        auto renderer = LLGL::RenderSystem::Load("OpenGL");

        window->SetTitle(
            windowDesc.title + L" ( " + std::wstring(renderer->GetName().begin(), renderer->GetName().end()) + L" )"
        );



        while (window->ProcessEvents() && !input->KeyPressed(LLGL::Key::Escape))
        {
            timer->MeasureTime();

            //std::cout << 1.0 / timer->GetDeltaTime() << std::endl;

            if (input->KeyDown(LLGL::Key::Num1))
                window->Show(false);
            if (input->KeyDown(LLGL::Key::Num2))
                window->Show(true);
            if (input->KeyDown(LLGL::Key::Num3))
                window->SetTitle(L"FOO BAR");
            if (input->KeyDown(LLGL::Key::Num4))
                window->SetTitle(L"LLGL Test 1");
            if (input->KeyDown(LLGL::Key::Num5))
                window->SetSize({ 300, 300 });

            #ifdef __APPLE__
            auto mousePos = input->GetMousePosition();
            std::wstringstream s;
            s << "X = " << mousePos.x << ", Y = " << mousePos.y;
            window->SetTitle(s.str());
            #endif

            if (input->KeyPressed(LLGL::Key::Right))
            {
                ++pos.x;
                window->SetPosition(pos);
            }
            if (input->KeyPressed(LLGL::Key::Left))
            {
                --pos.x;
                window->SetPosition(pos);
            }
            if (input->KeyPressed(LLGL::Key::Up))
            {
                --pos.y;
                window->SetPosition(pos);
            }
            if (input->KeyPressed(LLGL::Key::Down))
            {
                ++pos.y;
                window->SetPosition(pos);
            }

        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
