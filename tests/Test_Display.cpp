/*
 * Test_Display.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Display.h>
#include <iostream>
#include <thread>
#include <chrono>


int main(int argc, char* argv[])
{
    try
    {
        for (std::size_t i = 0; auto display = LLGL::Display::Get(i); ++i)
        {
            auto displayOffset  = display->GetOffset();
            auto displayMode    = display->GetDisplayMode();
            auto displayName    = display->GetDeviceName();

            std::cout << "Display: \"" << displayName.c_str() << "\"" << std::endl;
            std::cout << "|-Primary = " << std::boolalpha << display->IsPrimary() << std::endl;
            std::cout << "|-X       = " << displayOffset.x << std::endl;
            std::cout << "|-Y       = " << displayOffset.y << std::endl;
            std::cout << "|-Width   = " << displayMode.resolution.width << std::endl;
            std::cout << "|-Height  = " << displayMode.resolution.height << std::endl;
            std::cout << "|-Hz      = " << displayMode.refreshRate << std::endl;
            std::cout << "|-Scale   = " << display->GetScale() << std::endl;

            std::cout << "`-Settings:" << std::endl;
            auto supportedModes = display->GetSupportedDisplayModes();

            for (std::size_t i = 0, n = supportedModes.size(); i < n; ++i)
            {
                const auto& mode = supportedModes[i];
                auto ratio = GetExtentRatio(mode.resolution);

                if (i + 1 < n)
                    std::cout << "  |-";
                else
                    std::cout << "  `-";

                std::cout << "Mode[" << i << "]:";
                std::cout << " Width = " << mode.resolution.width;
                std::cout << ", Height = " << mode.resolution.height;
                std::cout << ", Hz = " << mode.refreshRate;
                std::cout << ", Ratio = " << ratio.width << ':' << ratio.height;
                std::cout << std::endl;
            }
        }

        #if 0
        if (auto display = LLGL::Display::QueryPrimary())
        {
            LLGL::DisplayModeDescriptor mode;
            mode.resolution.width = 1024;
            mode.resolution.height = 768;
            display->SetDisplayMode(mode);

            std::this_thread::sleep_for(std::chrono::seconds(1));
            display->ResetDisplayMode();
        }

        std::cout << "Wainting";
        for (int i = 0; i < 5; ++i)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << '.';
        }
        std::cout << std::endl;
        #endif
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
