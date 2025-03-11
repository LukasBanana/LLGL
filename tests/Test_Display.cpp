/*
 * Test_Display.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Display.h>
#include <LLGL/Log.h>
#include <thread>
#include <chrono>


int main(int argc, char* argv[])
{
    LLGL::Log::RegisterCallbackStd();

    // Test display information queries
    for (std::size_t i = 0; auto display = LLGL::Display::Get(i); ++i)
    {
        auto displayOffset  = display->GetOffset();
        auto displayMode    = display->GetDisplayMode();
        auto displayName    = display->GetDeviceName();

        LLGL::Log::Printf("Display: \"%s\"\n", displayName.c_str());
        LLGL::Log::Printf("|-Primary = %s\n", display->IsPrimary() ? "yes" : "no");
        LLGL::Log::Printf("|-X       = %d\n", displayOffset.x);
        LLGL::Log::Printf("|-Y       = %d\n", displayOffset.y);
        LLGL::Log::Printf("|-Width   = %u\n", displayMode.resolution.width);
        LLGL::Log::Printf("|-Height  = %u\n", displayMode.resolution.height);
        LLGL::Log::Printf("|-Hz      = %u\n", displayMode.refreshRate);
        LLGL::Log::Printf("|-Scale   = %f\n", display->GetScale());

        LLGL::Log::Printf("`-Settings:\n");
        auto supportedModes = display->GetSupportedDisplayModes();

        for (std::size_t i = 0, n = supportedModes.size(); i < n; ++i)
        {
            const auto& mode = supportedModes[i];
            auto ratio = GetExtentRatio(mode.resolution);

            LLGL::Log::Printf(i + 1 < n ? "  |-" : "  `-");
            LLGL::Log::Printf("Mode[%zu]:", i);
            LLGL::Log::Printf(" Width = %u", mode.resolution.width);
            LLGL::Log::Printf(", Height = %u", mode.resolution.height);
            LLGL::Log::Printf(", Hz = %u", mode.refreshRate);
            LLGL::Log::Printf(", Ratio = %u:%u\n", ratio.width, ratio.height);
        }
    }

    // Test changing display mode
    #if 0
    if (auto display = LLGL::Display::QueryPrimary())
    {
        LLGL::DisplayMode mode;
        mode.resolution.width = 1024;
        mode.resolution.height = 768;
        display->SetDisplayMode(mode);

        std::this_thread::sleep_for(std::chrono::seconds(1));
        display->ResetDisplayMode();
    }

    LLGL::Log::Printf("Wainting");
    for (int i = 0; i < 5; ++i)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        LLGL::Log::Printf(".");
    }
    LLGL::Log::Printf("\n");
    #endif

    // Test cursor position
    LLGL::Display::SetCursorPosition({ 10, 42 });
    auto cursorPos = LLGL::Display::GetCursorPosition();
    LLGL::Log::Printf("CursorPosition = (%d, %d)\n", cursorPos.x, cursorPos.y);

    #ifdef _WIN32
    system("pause");
    #endif

    return 0;
}
