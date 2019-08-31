/*
 * Test_Window.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/LLGL.h>
#include <memory>
#include <iostream>
#include <string>
#include <sstream>


static void printWindowSize(LLGL::Window& wnd)
{
    std::wcout << L"window: \"" << wnd.GetTitle() << L"\"" << std::endl;
    auto s = wnd.GetSize(true);
    std::wcout << "  content size = " << s.width << " x " << s.height << std::endl;
    s = wnd.GetSize(false);
    std::wcout << "  frame size   = " << s.width << " x " << s.height << std::endl;
};

static void printWindowPos(LLGL::Window& wnd)
{
    auto p = wnd.GetPosition();
    std::wcout << "window pos: x = " << p.x << ", y = " << p.y << std::endl;
}

class WindowEventHandler : public LLGL::Window::EventListener
{
public:
    void OnResize(LLGL::Window& sender, const LLGL::Extent2D& size) override
    {
        std::wcout << "OnResize: " << size.width << " x " << size.height << std::endl;
        printWindowSize(sender);
    }
};

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

        window->AddEventListener(std::make_shared<WindowEventHandler>());

        auto timer = LLGL::Timer::Create();

        auto pos = window->GetPosition();

        printWindowSize(*window);

        try
        {
            auto renderer = LLGL::RenderSystem::Load("OpenGL");

            window->SetTitle(
                windowDesc.title + L" ( " + std::wstring(renderer->GetName().begin(), renderer->GetName().end()) + L" )"
            );

            /*#ifdef __APPLE__
            LLGL::VideoModeDescriptor videoMode;
            {
                videoMode.fullscreen = true;
            }
            LLGL::Desktop::SetVideoMode(videoMode);
            #endif*/
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }

        LLGL::Extent2D desktopSize;
        if (auto display = LLGL::Display::InstantiatePrimary())
            desktopSize = display->GetDisplayMode().resolution;

        std::wcout << "Screen Width = " << desktopSize.width << ", Screen Height = " << desktopSize.height << std::endl;

        while (window->ProcessEvents() && !input->KeyPressed(LLGL::Key::Escape))
        {
            timer->MeasureTime();

            //std::cout << 1.0 / timer->GetDeltaTime() << std::endl;

            #ifdef __APPLE__
            if (input->KeyDown(LLGL::Key::D1))
                window->Show(false);
            if (input->KeyDown(LLGL::Key::D2))
                window->Show(true);
            #endif
            if (input->KeyDown(LLGL::Key::D3))
                window->SetTitle(L"FOO BAR");
            if (input->KeyDown(LLGL::Key::D4))
                window->SetTitle(L"LLGL Test 1");
            if (input->KeyDown(LLGL::Key::D5))
                window->SetSize({ 300, 300 });

            if (input->KeyDown(LLGL::Key::LButton))
                window->SetTitle(L"LButton Down");
            if (input->KeyDown(LLGL::Key::RButton))
                window->SetTitle(L"RButton Down");
            if (input->KeyDown(LLGL::Key::MButton))
                window->SetTitle(L"MButton Down");

            #ifdef __APPLE__

            auto mousePos = input->GetMousePosition();
            std::wstringstream s;
            s << "X = " << mousePos.x << ", Y = " << mousePos.y;
            //window->SetTitle(s.str());

            s << ", Screen Width = " << desktopSize.width << ", Screen Height = " << desktopSize.height;
            window->SetTitle(s.str());

            /*auto wheel = input->GetWheelMotion();
            if (wheel != 0)
            {
                pos.y += wheel;
                window->SetPosition(pos);
            }*/

            #endif

            if (input->KeyPressed(LLGL::Key::Right) && pos.x < 1920)
            {
                ++pos.x;
                window->SetPosition(pos);
                printWindowPos(*window);
            }
            if (input->KeyPressed(LLGL::Key::Left) && pos.x > 0)
            {
                --pos.x;
                window->SetPosition(pos);
                printWindowPos(*window);
            }
            if (input->KeyPressed(LLGL::Key::Up) && pos.y > 0)
            {
                --pos.y;
                window->SetPosition(pos);
                printWindowPos(*window);
            }
            if (input->KeyPressed(LLGL::Key::Down) && pos.y < 768)
            {
                ++pos.y;
                window->SetPosition(pos);
                printWindowPos(*window);
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
