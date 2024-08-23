/*
 * NativeHandle.h (OpenGL)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_OPENGL_NATIVE_HANDLE_H
#define LLGL_OPENGL_NATIVE_HANDLE_H


#include <LLGL/Platform/Platform.h>

#if defined(LLGL_OS_WIN32)
#   include <LLGL/Backend/OpenGL/Win32/Win32NativeHandle.h>
#elif defined(LLGL_OS_MACOS)
#   include <LLGL/Backend/OpenGL/MacOS/MacOSNativeHandle.h>
#elif defined(LLGL_OS_LINUX)
#   include <LLGL/Backend/OpenGL/Linux/LinuxNativeHandle.h>
#elif defined(LLGL_OS_IOS)
#   include <LLGL/Backend/OpenGL/IOS/IOSNativeHandle.h>
#elif defined(LLGL_OS_ANDROID)
#   include <LLGL/Backend/OpenGL/Android/AndroidNativeHandle.h>
#elif defined(LLGL_OS_WASM)
#   include <LLGL/Backend/OpenGL/Wasm/WasmNativeHandle.h>
#endif


#endif



// ================================================================================
