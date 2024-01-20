/*
 * Win32NativeHandle.h (OpenGL)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_OPENGL_WIN32_NATIVE_HANDLE_H
#define LLGL_OPENGL_WIN32_NATIVE_HANDLE_H


#include <Windows.h>
#include <GL/gl.h>
#include <GL/wglext.h>


namespace LLGL
{

namespace OpenGL
{


/**
\brief Windows native handle structure for the OpenGL render system.
\see RenderSystem::GetNativeHandle
\see RenderSystemDescriptor::nativeHandle
*/
struct RenderSystemNativeHandle
{
    HGLRC context;
};


} // /namespace OpenGL

} // /namespace LLGL


#endif



// ================================================================================
