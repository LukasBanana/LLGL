/*
 * MacOSNativeHandle.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MACOS_NATIVE_HANDLE_H
#define LLGL_MACOS_NATIVE_HANDLE_H


#include <Cocoa/Cocoa.h>


namespace LLGL
{


//! MacOS native handle structure.
struct NativeHandle
{
    NSWindow* window;
};


} // /namespace LLGL


#endif



// ================================================================================
