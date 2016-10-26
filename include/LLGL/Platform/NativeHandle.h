/*
 * NativeHandle.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_NATIVE_HANDLE_H
#define LLGL_NATIVE_HANDLE_H


#if defined(_WIN32)
#   include "Win32/Win32NativeHandle.h"
#elif defined(__APPLE__)
#   include "MacOS/MacOSNativeHandle.h"
#elif defined(__linux__)
#   include "Linux/LinuxNativeHandle.h"
#endif


#endif



// ================================================================================
