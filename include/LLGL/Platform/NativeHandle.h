/*
 * NativeHandle.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_NATIVE_HANDLE_H
#define LLGL_NATIVE_HANDLE_H


#include <LLGL/Platform/Platform.h>

#if defined(LLGL_OS_WIN32)
#   include <LLGL/Platform/Win32/Win32NativeHandle.h>
#elif defined(LLGL_OS_MACOS)
#   include <LLGL/Platform/MacOS/MacOSNativeHandle.h>
#elif defined(LLGL_OS_LINUX)
#   include <LLGL/Platform/Linux/LinuxNativeHandle.h>
#elif defined(LLGL_OS_IOS)
#   include <LLGL/Platform/IOS/IOSNativeHandle.h>
#elif defined(LLGL_OS_ANDROID)
#   include <LLGL/Platform/Android/AndroidNativeHandle.h>
#endif


#endif



// ================================================================================
