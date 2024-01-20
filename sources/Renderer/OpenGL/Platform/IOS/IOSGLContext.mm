/*
 * IOSGLContext.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "IOSGLContext.h"
#include "../../../CheckedCast.h"
#include "../../../StaticAssertions.h"
#include "../../../../Core/CoreUtils.h"
#include "../../../../Core/Exception.h"
#include <LLGL/RendererConfiguration.h>
#include <LLGL/Backend/OpenGL/NativeHandle.h>


namespace LLGL
{


/*
 * GLContext class
 */

LLGL_ASSERT_STDLAYOUT_STRUCT( OpenGL::RenderSystemNativeHandle );

std::unique_ptr<GLContext> GLContext::Create(
    const GLPixelFormat&                pixelFormat,
    const RendererConfigurationOpenGL&  profile,
    Surface&                            /*surface*/,
    GLContext*                          sharedContext,
    const ArrayView<char>&              customNativeHandle)
{
    IOSGLContext* sharedContextEGL = (sharedContext != nullptr ? LLGL_CAST(IOSGLContext*, sharedContext) : nullptr);
    return MakeUnique<IOSGLContext>(pixelFormat, profile, sharedContextEGL);
}


/*
 * IOSGLContext class
 */

IOSGLContext::IOSGLContext(
    const GLPixelFormat&                pixelFormat,
    const RendererConfigurationOpenGL&  profile,
    IOSGLContext*                       sharedContext)
:
    pixelFormat_ { pixelFormat }
{
    CreateContext(profile, sharedContext);
}

IOSGLContext::~IOSGLContext()
{
    DeleteContext();
}

int IOSGLContext::GetSamples() const
{
    return pixelFormat_.samples;
}

bool IOSGLContext::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(OpenGL::RenderSystemNativeHandle))
    {
        auto* nativeHandleGL = reinterpret_cast<OpenGL::RenderSystemNativeHandle*>(nativeHandle);
        nativeHandleGL->context = context_;
        return true;
    }
    return false;
}


/*
 * ======= Private: =======
 */

bool IOSGLContext::SetSwapInterval(int /*interval*/)
{
    return true; // dummy
}

static EAGLRenderingAPI GetOpenGLESApi(const RendererConfigurationOpenGL& profile)
{
    if (profile.majorVersion == 2)
        return kEAGLRenderingAPIOpenGLES2;
    else
        return kEAGLRenderingAPIOpenGLES3; // default to GLES3
}

void IOSGLContext::CreateContext(
    const RendererConfigurationOpenGL&  profile,
    IOSGLContext*                       sharedContext)
{
    /* Initialize EAGL with OpenGLES API */
    EAGLRenderingAPI apiEAGL = GetOpenGLESApi(profile);

    if (sharedContext != nullptr)
    {
        EAGLContext* sharedEAGLContext = sharedContext->GetEAGLContext();
        context_ = [[EAGLContext alloc] initWithAPI:apiEAGL sharegroup:[sharedEAGLContext sharegroup]];
    }
    else
        context_ = [[EAGLContext alloc] initWithAPI:apiEAGL];

    if (context_ == nil)
    {
        LLGL_TRAP(
            "EAGLContext initialization with OpenGLES %s API failed",
            (apiEAGL == kEAGLRenderingAPIOpenGLES3 ? "3" : "2")
        );
    }

    /* Make new context current */
    [EAGLContext setCurrentContext:context_];
}

void IOSGLContext::DeleteContext()
{
    [context_ release];
}


} // /namespace LLGL



// ================================================================================
