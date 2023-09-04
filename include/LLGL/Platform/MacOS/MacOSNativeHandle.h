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


/**
\brief MacOS native handle structure.
\see Window::GetNativeHandle
\see WindowDescriptor::windowContext
*/
struct NativeHandle
{
    //! NSWindow object for top level windows.
    NSWindow*   window;

    //! NSView object for borderless windows that have a parent window.
    NSView*     view;
};


} // /namespace LLGL


#endif



// ================================================================================
