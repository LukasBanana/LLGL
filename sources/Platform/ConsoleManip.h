/*
 * ConsoleManip.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_CONSOLE_MANIP_H
#define LLGL_CONSOLE_MANIP_H


#include <LLGL/Log.h>


#define LLGL_LOG_GET_R(COLOR) (((COLOR) >> 24) & 0xFF)
#define LLGL_LOG_GET_G(COLOR) (((COLOR) >> 16) & 0xFF)
#define LLGL_LOG_GET_B(COLOR) (((COLOR) >>  8) & 0xFF)


namespace LLGL
{

// Namespace with abstract platform functions for console manipulation
namespace ConsoleManip
{


void GetConsoleColors(Log::ReportType type, Log::ColorCodes& outColors);
void SetConsoleColors(Log::ReportType type, const Log::ColorCodes& inColors);

int FormatColorCodesVT100(char* outFormat, int formatSize, const Log::ColorCodes& colors);

long GetColorFlagsFromRGB(int r, int g, int b);

class ScopedConsoleColors
{

        Log::ReportType type_;
        Log::ColorCodes oldColors_;

    public:

        inline ScopedConsoleColors(Log::ReportType type, const Log::ColorCodes& newColors) :
            type_ { type }
        {
            GetConsoleColors(type, oldColors_);
            SetConsoleColors(type, newColors);
        }

        inline ~ScopedConsoleColors()
        {
            SetConsoleColors(type_, oldColors_);
        }

};


} // /nameapace ConsoleManip

} // /namespace LLGL


#endif



// ================================================================================
