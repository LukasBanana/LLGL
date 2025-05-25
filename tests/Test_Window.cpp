/*
 * Test_Window.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/LLGL.h>
#include <memory>
#include <string>


static void printWindowSize(LLGL::Window& wnd)
{
    LLGL::Log::Printf("window: \"%s\"\n", wnd.GetTitle().c_str());
    auto s = wnd.GetSize(true);
    LLGL::Log::Printf("  content size = %u x %u\n", s.width, s.height);
    s = wnd.GetSize(false);
    LLGL::Log::Printf("  frame size   = %u x %u\n", s.width, s.height);
};

static void printWindowPos(LLGL::Window& wnd)
{
    auto p = wnd.GetPosition();
    LLGL::Log::Printf("window pos: x = %d, y = %d\n", p.x, p.y);
}

class WindowEventHandler : public LLGL::Window::EventListener
{
public:
    void OnResize(LLGL::Window& sender, const LLGL::Extent2D& size) override
    {
        LLGL::Log::Printf("OnResize: %u x %u\n", size.width, size.height);
        printWindowSize(sender);
    }
};

int main()
{
    try
    {
        LLGL::Log::RegisterCallbackStd();

        // Create window
        LLGL::WindowDescriptor windowDesc;

        windowDesc.title    = "LLGL Test 1";
        windowDesc.flags    = LLGL::WindowFlags::Visible | LLGL::WindowFlags::Centered | LLGL::WindowFlags::Resizable;
        windowDesc.size     = { 640, 480 };

        auto window = LLGL::Window::Create(windowDesc);

        LLGL::Input input{ *window };

        window->AddEventListener(std::make_shared<WindowEventHandler>());

        auto pos = window->GetPosition();

        printWindowSize(*window);

        try
        {
            auto renderer = LLGL::RenderSystem::Load("OpenGL");

            window->SetTitle(
                std::string(windowDesc.title.c_str()) + " ( " + std::string(renderer->GetName()) + " )"
            );
        }
        catch (const std::exception& e)
        {
            LLGL::Log::Errorf("%s\n", e.what());
        }

        LLGL::Extent2D desktopSize;
        if (auto display = LLGL::Display::GetPrimary())
            desktopSize = display->GetDisplayMode().resolution;

        LLGL::Log::Printf("Screen Width = %u, Screen Height = %u\n", desktopSize.width, desktopSize.height);

        while (LLGL::Surface::ProcessEvents() && !window->HasQuit() && !input.KeyPressed(LLGL::Key::Escape))
        {
            if (input.KeyPressed(LLGL::Key::C))
                LLGL::Display::SetCursorPosition(LLGL::Offset2D{ 150, 150 });

            #ifdef __APPLE__
            if (input.KeyDown(LLGL::Key::D1))
                window->Show(false);
            if (input.KeyDown(LLGL::Key::D2))
                window->Show(true);
            #endif
            if (input.KeyDown(LLGL::Key::D3))
                window->SetTitle("FOO BAR");
            if (input.KeyDown(LLGL::Key::D4))
                window->SetTitle("LLGL Test 1");
            if (input.KeyDown(LLGL::Key::D5))
                window->SetSize({ 300, 300 });

            if (input.KeyDown(LLGL::Key::LButton))
                window->SetTitle("LButton Down");
            if (input.KeyDown(LLGL::Key::RButton))
                window->SetTitle("RButton Down");
            if (input.KeyDown(LLGL::Key::MButton))
                window->SetTitle("MButton Down");

            if (input.KeyPressed(LLGL::Key::Right) && pos.x < 1920)
            {
                ++pos.x;
                window->SetPosition(pos);
                printWindowPos(*window);
            }
            if (input.KeyPressed(LLGL::Key::Left) && pos.x > 0)
            {
                --pos.x;
                window->SetPosition(pos);
                printWindowPos(*window);
            }
            if (input.KeyPressed(LLGL::Key::Up) && pos.y > 0)
            {
                --pos.y;
                window->SetPosition(pos);
                printWindowPos(*window);
            }
            if (input.KeyPressed(LLGL::Key::Down) && pos.y < 768)
            {
                ++pos.y;
                window->SetPosition(pos);
                printWindowPos(*window);
            }

            input.Reset();
        }
    }
    catch (const std::exception& e)
    {
        LLGL::Log::Errorf("%s\n", e.what());
    }

    return 0;
}
