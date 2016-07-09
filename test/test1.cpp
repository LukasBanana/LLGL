/*
 * test1.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/LLGL.h>
#include <memory>
#include <iostream>



int main()
{
    // Create window
    LLGL::WindowDesc windowDesc;
    
    windowDesc.title    = L"LLGL Test 1";
    windowDesc.visible  = true;
    windowDesc.centered = true;
    windowDesc.width    = 640;
    windowDesc.height   = 480;
    
    auto window = LLGL::Window::Create(windowDesc);
    
    auto input = std::make_shared<LLGL::Input>();
    window->AddListener(input);
    
    auto timer = LLGL::Timer::Create();
    
    
    while (window->ProcessEvents() && !input->KeyPressed(LLGL::Key::Escape))
    {
        timer->MeasureTime();
        
        //std::cout << 1.0 / timer->GetDeltaTime() << std::endl;
        
        if (input->KeyPressed(LLGL::Key::A))
            std::cout << "A" << std::endl;
        if (input->KeyPressed(LLGL::Key::B))
            std::cout << "B" << std::endl;
        if (input->KeyPressed(LLGL::Key::C))
            std::cout << "C" << std::endl;
        
        
    }

    return 0;
}
