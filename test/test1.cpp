/*
 * test1.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/LLGL.h>
#include <memory>



int main()
{
    // Create window
    LLGL::WindowDesc windowDesc;

    windowDesc.title = L"LLGL Test 1";
    windowDesc.visible = true;
    windowDesc.centered = true;
    windowDesc.width = 640;
    windowDesc.height = 480;

    auto window = LLGL::Window::Create(windowDesc);

    auto input = std::make_shared<LLGL::Input>();
    window->AddListener(input);


    while (window->ProcessEvents() && !input->KeyPressed(LLGL::Key::Escape))
    {




        input->Reset();
    }

    return 0;
}
