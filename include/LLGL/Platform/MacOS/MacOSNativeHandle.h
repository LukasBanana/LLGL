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
    /**
    \brief Generic \c NSResponder object that must be either of type \c NSWindow or \c NSView.
    \remarks When a SwapChain is created, the responder is interpreted as:
    - \b Top-level window if it points to an \c NSWindow, in which case the respective \c MTKView (Metal) or \c GLKView (OpenGL) will \e replace its content view.
    - \b Subview if it points to an \c NSView, in which case the respective \c MTKView (Metal) or \c GLKView (OpenGL) will be \e added as a subview.
    \see WindowDescriptor::windowContext
    */
    NSResponder* responder;
};


} // /namespace LLGL


#endif



// ================================================================================
