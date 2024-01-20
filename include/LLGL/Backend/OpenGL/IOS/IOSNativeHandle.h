/*
 * IOSNativeHandle.h (OpenGL)
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_OPENGL_IOS_NATIVE_HANDLE_H
#define LLGL_OPENGL_IOS_NATIVE_HANDLE_H


#include <OpenGLES/EAGL.h>


namespace LLGL
{

namespace OpenGL
{


/**
\brief iOS native handle structure for the OpenGL render system.
\see RenderSystem::GetNativeHandle
\see RenderSystemDescriptor::nativeHandle
*/
struct RenderSystemNativeHandle
{
    EAGLContext* context;
};


} // /namespace OpenGL

} // /namespace LLGL


#endif



// ================================================================================
