/*
 * MacOSNativeHandle.h (OpenGL)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_OPENGL_MACOS_NATIVE_HANDLE_H
#define LLGL_OPENGL_MACOS_NATIVE_HANDLE_H

#ifdef __OBJC__
#include <Cocoa/Cocoa.h>
#endif


namespace LLGL
{

namespace OpenGL
{


/**
\brief macOS native handle structure for the OpenGL render system.
\see RenderSystem::GetNativeHandle
\see RenderSystemDescriptor::nativeHandle
*/
struct RenderSystemNativeHandle
{
#ifdef __OBJC__
    NSOpenGLContext* context;
#else
    void* context;
#endif
};


} // /namespace OpenGL

} // /namespace LLGL


#endif



// ================================================================================
