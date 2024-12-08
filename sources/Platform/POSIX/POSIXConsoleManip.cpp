/*
 * POSIXConsoleManip.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../ConsoleManip.h"
#include <unistd.h>
#include <stdio.h>


namespace LLGL
{

namespace ConsoleManip
{

    
class PosixTerminalState
{

    public:

        PosixTerminalState(FILE* fileHandle) :
            fileHandle_           { fileHandle                            },
            isAttachedToTerminal_ { (::isatty(::fileno(fileHandle)) != 0) }
        {
        }

        void GetConsoleColors(Log::ColorCodes& outColors)
        {
            outColors = currentColors_;
        }

        void SetConsoleColors(const Log::ColorCodes& inColors)
        {
            if (isAttachedToTerminal_)
            {
                constexpr int formatSize = 128;
                char format[formatSize];
                (void)FormatColorCodesVT100(format, formatSize, inColors);
                ::fprintf(fileHandle_, "%s", format);
                currentColors_ = inColors;
            }
        }

    private:

        FILE*           fileHandle_             = nullptr;
        const bool      isAttachedToTerminal_   = false;
        Log::ColorCodes currentColors_          = { Log::ColorFlags::Default, Log::ColorFlags::Default };

};

static PosixTerminalState g_stdOutState{ stdout };
static PosixTerminalState g_stdErrState{ stderr };

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
