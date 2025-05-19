/*
 * LinuxGLContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../GLContext.h"
#include "../../Ext/GLExtensions.h"
#include "../../Ext/GLExtensionLoader.h"
#include "../../GLCore.h"
#include "../../../CheckedCast.h"
#include "../../../StaticAssertions.h"
#include "../../../RenderSystemUtils.h"
#include "../../../../Core/CoreUtils.h"
#include "../../../../Core/Assertion.h"
#include "../../../../Platform/Linux/LinuxDisplay.h"
#include <LLGL/Backend/OpenGL/NativeHandle.h>
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Log.h>
#include <algorithm>

#include "LinuxGLContextWayland.h"
#include "LinuxGLContextX11.h"


namespace LLGL
{


#ifndef GLX_CONTEXT_MAJOR_VERSION_ARB
#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#endif

#ifndef GLX_CONTEXT_MINOR_VERSION_ARB
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
#endif

typedef GLXContext (*GXLCREATECONTEXTATTRIBARBPROC)(::Display*, GLXFBConfig, GLXContext, Bool, const int*);


/*
 * GLContext class
 */

LLGL_ASSERT_STDLAYOUT_STRUCT( OpenGL::RenderSystemNativeHandle );

std::unique_ptr<GLContext> GLContext::Create(
    const GLPixelFormat&                pixelFormat,
    const RendererConfigurationOpenGL&  profile,
    Surface&                            surface,
    GLContext*                          sharedContext,
    const ArrayView<char>&              customNativeHandle)
{
    LLGL::NativeHandle nativeHandle = {};
    surface.GetNativeHandle(&nativeHandle, sizeof(nativeHandle));

    if (nativeHandle.type == NativeType::Wayland)
    {
        #if LLGL_LINUX_ENABLE_WAYLAND

        LinuxGLContextWayland* sharedContextGLX = (sharedContext != nullptr ? LLGL_CAST(LinuxGLContextWayland*, sharedContext) : nullptr);

        return MakeUnique<LinuxGLContextWayland>(
            pixelFormat, profile, surface, sharedContextGLX,
            GetRendererNativeHandle<OpenGL::RenderSystemNativeHandle>(customNativeHandle)
        );

        #else

        LLGL_TRAP("Native handle type is Wayland but LLGL was built without LLGL_LINUX_ENABLE_WAYLAND");

        #endif
    }
    else
    {
        LinuxGLContextX11* sharedContextGLX = (sharedContext != nullptr ? LLGL_CAST(LinuxGLContextX11*, sharedContext) : nullptr);

        return MakeUnique<LinuxGLContextX11>(
            pixelFormat, profile, surface, sharedContextGLX,
            GetRendererNativeHandle<OpenGL::RenderSystemNativeHandle>(customNativeHandle)
        );
    }
}


} // /namespace LLGL



// ================================================================================
