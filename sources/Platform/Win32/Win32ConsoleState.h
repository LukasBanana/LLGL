/*
 * Win32ConsoleState.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WIN32_CONSOLE_STATE_H
#define LLGL_WIN32_CONSOLE_STATE_H


#include "../ConsoleManip.h"
#include <Windows.h>
#include <stdio.h>


namespace LLGL
{

namespace ConsoleManip
{


class Win32ConsoleState
{

    public:

        Win32ConsoleState(HANDLE outputHandle, FILE* fileHandle);

        void GetConsoleColors(Log::ColorCodes& outColors);
        void SetConsoleColors(const Log::ColorCodes& inColors);

    private:

        void GetCurrentLegacyConsoleColors(Log::ColorCodes& outColors);
        void SetLegacyConsoleColors(const Log::ColorCodes& inColors);
        void SetVirtualConsoleColors(const Log::ColorCodes& inColors);

    private:

        HANDLE          outputHandle_           = nullptr;
        FILE*           fileHandle_             = nullptr;
        DWORD           initialConsoleMode_     = 0;
        bool            isConsoleMode_          = false;
        bool            isVT100ModeSupported_   = false;
        Log::ColorCodes initialColors_;
        Log::ColorCodes currentColors_;

};


} // /namespace ConsoleManip

} // /namespace LLGL


#endif



// ================================================================================
