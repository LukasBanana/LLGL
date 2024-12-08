/*
 * Win32ConsoleManip.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Win32ConsoleState.h"
#include "../ConsoleManip.h"
#include <Windows.h>
#include <stdio.h>


namespace LLGL
{

namespace ConsoleManip
{


Win32ConsoleState::Win32ConsoleState(HANDLE outputHandle, FILE* fileHandle) :
    outputHandle_  { outputHandle                                       },
    fileHandle_    { fileHandle                                         },
    initialColors_ { Log::ColorFlags::Default, Log::ColorFlags::Default },
    currentColors_ { Log::ColorFlags::Default, Log::ColorFlags::Default }
{
    /*
    Determine whether standard output points to the console.
    Otherwise, don't allow color coding when outputting to a pipe or file.
    */
    DWORD stdOutFileType = ::GetFileType(outputHandle);

    DWORD initialConsoleMode = 0;
    isConsoleMode_ = (::GetConsoleMode(outputHandle, &initialConsoleMode) != FALSE && stdOutFileType == FILE_TYPE_CHAR);

    if (isConsoleMode_)
    {
        /* Check if virtual console mode is supported to enable extended color codes */
        isVT100ModeSupported_ = (::SetConsoleMode(outputHandle, initialConsoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) == TRUE);

        /* Otherwise, use legacy console attributes */
        if (!isVT100ModeSupported_)
        {
            GetCurrentLegacyConsoleColors(initialColors_);
            currentColors_ = initialColors_;
        }
    }
}

void Win32ConsoleState::GetConsoleColors(Log::ColorCodes& outColors)
{
    outColors = currentColors_;
}

void Win32ConsoleState::SetConsoleColors(const Log::ColorCodes& inColors)
{
    if (isConsoleMode_)
    {
        if (isVT100ModeSupported_)
            SetVirtualConsoleColors(inColors);
        else
            SetLegacyConsoleColors(inColors);
        currentColors_ = inColors;
    }
}

void Win32ConsoleState::GetCurrentLegacyConsoleColors(Log::ColorCodes& outColors)
{
    CONSOLE_SCREEN_BUFFER_INFO bufferInfo = {};
    ::GetConsoleScreenBufferInfo(outputHandle_, &bufferInfo);

    outColors.textFlags = 0;

    if ((bufferInfo.wAttributes & FOREGROUND_RED) != 0)
        outColors.textFlags |= Log::ColorFlags::Red;
    if ((bufferInfo.wAttributes & FOREGROUND_GREEN) != 0)
        outColors.textFlags |= Log::ColorFlags::Green;
    if ((bufferInfo.wAttributes & FOREGROUND_BLUE) != 0)
        outColors.textFlags |= Log::ColorFlags::Blue;
    if ((bufferInfo.wAttributes & FOREGROUND_INTENSITY) != 0)
        outColors.textFlags |= Log::ColorFlags::Bright;

    outColors.backgroundFlags = 0;

    if ((bufferInfo.wAttributes & BACKGROUND_RED) != 0)
        outColors.backgroundFlags |= Log::ColorFlags::Red;
    if ((bufferInfo.wAttributes & BACKGROUND_GREEN) != 0)
        outColors.backgroundFlags |= Log::ColorFlags::Green;
    if ((bufferInfo.wAttributes & BACKGROUND_BLUE) != 0)
        outColors.backgroundFlags |= Log::ColorFlags::Blue;
    if ((bufferInfo.wAttributes & BACKGROUND_INTENSITY) != 0)
        outColors.backgroundFlags |= Log::ColorFlags::Bright;
}

void Win32ConsoleState::SetLegacyConsoleColors(const Log::ColorCodes& inColors)
{
    CONSOLE_SCREEN_BUFFER_INFO bufferInfo = {};
    ::GetConsoleScreenBufferInfo(outputHandle_, &bufferInfo);

    WORD attribs = 0;

    if (long textFlags = inColors.textFlags)
    {
        if (textFlags == Log::ColorFlags::Default)
            textFlags = initialColors_.textFlags;

        if ((textFlags & Log::ColorFlags::FullRGB) != 0)
            textFlags = GetColorFlagsFromRGB(LLGL_LOG_GET_R(textFlags), LLGL_LOG_GET_G(textFlags), LLGL_LOG_GET_B(textFlags));

        if ((textFlags & Log::ColorFlags::Red) != 0)
            attribs |= FOREGROUND_RED;
        if ((textFlags & Log::ColorFlags::Green) != 0)
            attribs |= FOREGROUND_GREEN;
        if ((textFlags & Log::ColorFlags::Blue) != 0)
            attribs |= FOREGROUND_BLUE;
        if ((textFlags & Log::ColorFlags::Bright) != 0)
            attribs |= FOREGROUND_INTENSITY;
    }
    else
    {
        /* Copy attributes for foreground from current console status */
        attribs |= (bufferInfo.wAttributes & (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY));
    }

    if (long backgroundFlags = inColors.backgroundFlags)
    {
        if (backgroundFlags == Log::ColorFlags::Default)
            backgroundFlags = initialColors_.backgroundFlags;

        if ((backgroundFlags & Log::ColorFlags::FullRGB) != 0)
            backgroundFlags = GetColorFlagsFromRGB(LLGL_LOG_GET_R(backgroundFlags), LLGL_LOG_GET_G(backgroundFlags), LLGL_LOG_GET_B(backgroundFlags));

        if ((backgroundFlags & Log::ColorFlags::Red) != 0)
            attribs |= BACKGROUND_RED;
        if ((backgroundFlags & Log::ColorFlags::Green) != 0)
            attribs |= BACKGROUND_GREEN;
        if ((backgroundFlags & Log::ColorFlags::Blue) != 0)
            attribs |= BACKGROUND_BLUE;
        if ((backgroundFlags & Log::ColorFlags::Bright) != 0)
            attribs |= BACKGROUND_INTENSITY;
    }
    else
    {
        /* Copy attributes for background from current console status */
        attribs |= (bufferInfo.wAttributes & (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY));
    }

    ::SetConsoleTextAttribute(outputHandle_, attribs);
}

void Win32ConsoleState::SetVirtualConsoleColors(const Log::ColorCodes& inColors)
{
    constexpr int formatSize = 128;
    char format[formatSize];
    (void)FormatColorCodesVT100(format, formatSize, inColors);
    ::fprintf(fileHandle_, "%s", format);
}


} // /namespace ConsoleManip

} // /namespace LLGL



// ================================================================================
