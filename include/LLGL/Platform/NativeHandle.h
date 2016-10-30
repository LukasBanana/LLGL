/*
 * NativeHandle.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_NATIVE_HANDLE_H
#define LLGL_NATIVE_HANDLE_H


#include "Platform.h"

#if defined(LLGL_OS_WIN32)
#   include "Win32/Win32NativeHandle.h"
#elif defined(LLGL_OS_MACOS)
#   include "MacOS/MacOSNativeHandle.h"
#elif defined(LLGL_OS_LINUX)
#   include "Linux/LinuxNativeHandle.h"
#elif defined(LLGL_OS_IOS)
#   include "IOS/IOSNativeHandle.h"
#endif


#endif



// ================================================================================
