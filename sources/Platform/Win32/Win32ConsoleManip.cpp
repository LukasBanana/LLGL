/*
 * Win32ConsoleManip.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../ConsoleManip.h"
#include "Win32ConsoleState.h"
#include <Windows.h>
#include <stdio.h>


namespace LLGL
{

namespace ConsoleManip
{


static Win32ConsoleState g_stdOutState{ GetStdHandle(STD_OUTPUT_HANDLE), stdout };
static Win32ConsoleState g_stdErrState{ GetStdHandle(STD_ERROR_HANDLE), stderr };

void GetConsoleColors(Log::ReportType type, Log::ColorCodes& outColors)
{
    if (type == Log::ReportType::Error)
        g_stdErrState.GetConsoleColors(outColors);
    else
        g_stdOutState.GetConsoleColors(outColors);
}

void SetConsoleColors(Log::ReportType type, const Log::ColorCodes& inColors)
{
    if (type == Log::ReportType::Error)
        g_stdErrState.SetConsoleColors(inColors);
    else
        g_stdOutState.SetConsoleColors(inColors);
}


} // /nameapace ConsoleManip

} // /namespace LLGL



// ================================================================================
