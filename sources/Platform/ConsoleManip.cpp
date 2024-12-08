/*
 * ConsoleManip.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "ConsoleManip.h"
#include "../Core/Assertion.h"
#include <stdio.h>


namespace LLGL
{

namespace ConsoleManip
{


static constexpr int k_virtualConsoleCodeReset      = 9;
static constexpr int k_virtualConsoleCodeBright     = 60;
static constexpr int k_virtualConsoleCodeForeground = 30;
static constexpr int k_virtualConsoleCodeBackground = 40;

static int FormatColorCodesVT100Base(char* outFormat, int formatSize, long colorFlags, int baseCode)
{
    int numCharsWritten = 0;

    #define APPEND_FORMAT(...)                                      \
        LLGL_ASSERT(numCharsWritten < formatSize);                  \
        numCharsWritten += ::snprintf(                              \
            outFormat + numCharsWritten,                            \
            static_cast<std::size_t>(formatSize - numCharsWritten), \
            __VA_ARGS__                                             \
        );

    if (colorFlags == Log::ColorFlags::Default)
    {
        /* Append code to reset to deafult */
        APPEND_FORMAT("\x1B[%dm", (baseCode + k_virtualConsoleCodeReset));
    }
    else
    {
        if ((colorFlags & Log::ColorFlags::Bold) != 0)
        {
            /* Append code for bold text */
            APPEND_FORMAT("\x1B[1m");
        }

        if ((colorFlags & Log::ColorFlags::Underline) != 0)
        {
            /* Append code for underline */
            APPEND_FORMAT("\x1B[4m");
        }

        if ((colorFlags & Log::ColorFlags::FullRGB) != 0)
        {
            APPEND_FORMAT(
                "\x1B[%d;2;%d;%d;%dm",
                (baseCode + 8),
                static_cast<int>(LLGL_LOG_GET_R(colorFlags)),
                static_cast<int>(LLGL_LOG_GET_G(colorFlags)),
                static_cast<int>(LLGL_LOG_GET_B(colorFlags))
            );
        }
        else
        {
            int code = ((colorFlags & Log::ColorFlags::Bright) != 0 ? baseCode + k_virtualConsoleCodeBright : baseCode);

            if ((colorFlags & Log::ColorFlags::Red) != 0)
                code += 1;
            if ((colorFlags & Log::ColorFlags::Green) != 0)
                code += 2;
            if ((colorFlags & Log::ColorFlags::Blue) != 0)
                code += 4;

            APPEND_FORMAT("\x1B[%dm", code);
        }
    }

    #undef APPEND_FORMAT

    return numCharsWritten;
}

int FormatColorCodesVT100(char* outFormat, int formatSize, const Log::ColorCodes& colors)
{
    int numCharsWritten = 0;

    if (colors.textFlags == Log::ColorFlags::Default && colors.backgroundFlags == Log::ColorFlags::Default)
    {
        /* Reset all attributes to default values */
        numCharsWritten += ::snprintf(outFormat, formatSize, "\x1B[0m");
    }
    else
    {
        /* Set attributes for text and background individually */
        if (colors.textFlags != 0)
        {
            numCharsWritten += FormatColorCodesVT100Base(
                outFormat + numCharsWritten,
                formatSize - numCharsWritten,
                colors.textFlags,
                k_virtualConsoleCodeForeground
            );
        }
        if (colors.backgroundFlags != 0)
        {
            numCharsWritten += FormatColorCodesVT100Base(
                outFormat + numCharsWritten,
                formatSize - numCharsWritten,
                colors.backgroundFlags,
                k_virtualConsoleCodeBackground
            );
        }
    }

    return numCharsWritten;
}

long GetColorFlagsFromRGB(int r, int g, int b)
{
    long flags = 0;

    if (r > 64)
        flags |= Log::ColorFlags::Red;
    if (g > 64)
        flags |= Log::ColorFlags::Green;
    if (b > 64)
        flags |= Log::ColorFlags::Blue;

    if (r > 128 + 64 ||
        g > 128 + 64 ||
        b > 128 + 64)
    {
        flags |= Log::ColorFlags::Bright;
    }

    return flags;
}


} // /nameapace ConsoleManip

} // /namespace LLGL



// ================================================================================
