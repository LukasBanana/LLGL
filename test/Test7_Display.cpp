/*
 * Test7_Display.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Display.h>
#include <iostream>


int main(int argc, char* argv[])
{
    auto displayList = LLGL::Display::QueryList();
    for (const auto& display : displayList)
    {
        auto displayOffset  = display->GetOffset();
        auto displayMode    = display->GetDisplayMode();

        std::wcout << "Display: \"" << display->GetDeviceName().c_str() << "\"" << std::endl;
        std::cout << "|-Primary = " << std::boolalpha << display->IsPrimary() << std::endl;
        std::cout << "|-X       = " << displayOffset.x << std::endl;
        std::cout << "|-Y       = " << displayOffset.y << std::endl;
        std::cout << "|-Width   = " << displayMode.resolution.width << std::endl;
        std::cout << "|-Height  = " << displayMode.resolution.height << std::endl;
        std::cout << "`-Hz      = " << displayMode.refreshRate << std::endl;

        std::cout << "`-Settings:" << std::endl;
        auto supportedModes = display->QuerySupportedDisplayModes();

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


    #ifdef _WIN32
    system("pause");
    #endif

    return 0;
}
