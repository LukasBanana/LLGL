/*
 * MacOSGLContext.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MacOSGLContext.h"
#include "../../../../Platform/MacOS/MacOSWindow.h"
#include "../../../../Core/CoreUtils.h"
#include "../../../../Core/Assertion.h"
#include "../../../../Core/Exception.h"
#include "../../../CheckedCast.h"
#include "../../../StaticAssertions.h"
#include "../../../TextureUtils.h"
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Backend/OpenGL/NativeHandle.h>
#include <LLGL/Log.h>


namespace LLGL
{


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
    MacOSGLContext* sharedContextGLNS = (sharedContext != nullptr ? LLGL_CAST(MacOSGLContext*, sharedContext) : nullptr);
    return MakeUnique<MacOSGLContext>(pixelFormat, profile, surface, sharedContextGLNS);
}


/*
 * MacOSGLContext class
 */

MacOSGLContext::MacOSGLContext(
    const GLPixelFormat&                pixelFormat,
    const RendererConfigurationOpenGL&  profile,
    Surface&                            surface,
    MacOSGLContext*                     sharedContext)
{
    if (!CreatePixelFormat(pixelFormat, profile))
        LLGL_TRAP("failed to find suitable OpenGL pixel format");

    CreateNSGLContext(sharedContext);
}

MacOSGLContext::~MacOSGLContext()
{
    DeleteNSGLContext();
}

int MacOSGLContext::GetSamples() const
{
    return samples_;
}

bool MacOSGLContext::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(OpenGL::RenderSystemNativeHandle))
    {
        auto* nativeHandleGL = static_cast<OpenGL::RenderSystemNativeHandle*>(nativeHandle);
        nativeHandleGL->context = ctx_;
        return true;
    }
    return false;
}

static NSOpenGLContext* g_currentNSGLContext;

void MacOSGLContext::MakeNSOpenGLContextCurrent(NSOpenGLContext* context)
{
    if (g_currentNSGLContext != context)
    {
        [context makeCurrentContext];
        g_currentNSGLContext = context;
    }
}


/*
 * ======= Private: =======
 */

bool MacOSGLContext::SetSwapInterval(int interval)
{
    #if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_14 && !LLGL_GL_ENABLE_OPENGL2X
    [ctx_ setValues:&interval forParameter:NSOpenGLContextParameterSwapInterval];
    #else
    [ctx_ setValues:&interval forParameter:NSOpenGLCPSwapInterval];
    #endif
    return true;
}

#if !LLGL_GL_ENABLE_OPENGL2X

static NSOpenGLPixelFormatAttribute TranslateNSOpenGLProfile(const RendererConfigurationOpenGL& profile)
{
    if (profile.contextProfile == OpenGLContextProfile::CompatibilityProfile)
    {
        /* Choose OpenGL compatibility profile */
        return NSOpenGLProfileVersionLegacy;
    }
    if (profile.contextProfile == OpenGLContextProfile::CoreProfile)
    {
        if ((profile.majorVersion == 0 && profile.minorVersion == 0) ||
            (profile.majorVersion == 4 && profile.minorVersion == 1))
        {
            /* Choose OpenGL 4.1 core profile (default) */
            return NSOpenGLProfileVersion4_1Core;
        }
        if (profile.majorVersion == 3 && profile.minorVersion == 2)
        {
            /* Choose OpenGL 3.2 core profile */
            return NSOpenGLProfileVersion3_2Core;
        }
    }
    
    LLGL_TRAP("failed to choose OpenGL profile (only compatibility profile, 3.2 core profile, and 4.1 core profile are supported)");
}

#endif // /!LLGL_GL_ENABLE_OPENGL2X

bool MacOSGLContext::CreatePixelFormat(const GLPixelFormat& pixelFormat, const RendererConfigurationOpenGL& profile)
{
    #if !LLGL_GL_ENABLE_OPENGL2X
    const NSOpenGLPixelFormatAttribute profileAttrib = TranslateNSOpenGLProfile(profile);
    #endif

    /* Find suitable pixel format (for samples > 0) */
    for (samples_ = std::max<int>(1, pixelFormat.samples); samples_ > 0; --samples_)
    {
        NSOpenGLPixelFormatAttribute attribs[] =
        {
            NSOpenGLPFAAccelerated,
            NSOpenGLPFADoubleBuffer,
            #if !LLGL_GL_ENABLE_OPENGL2X
            NSOpenGLPFAOpenGLProfile,   profileAttrib,
            #endif
            NSOpenGLPFADepthSize,       static_cast<NSOpenGLPixelFormatAttribute>(pixelFormat.depthBits),
            NSOpenGLPFAStencilSize,     static_cast<NSOpenGLPixelFormatAttribute>(pixelFormat.stencilBits),
            NSOpenGLPFAColorSize,       24,
            NSOpenGLPFAAlphaSize,       8,
            //NSOpenGLPFAMultisample,
            NSOpenGLPFASampleBuffers,   (samples_ > 1 ? 1u : 0u),
            NSOpenGLPFASamples,         static_cast<NSOpenGLPixelFormatAttribute>(samples_),
            0
        };

        /* Allocate NS-OpenGL pixel format */
        pixelFormat_ = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
        if (pixelFormat_)
        {
            SetDefaultColorFormat();
            DeduceDepthStencilFormat(pixelFormat.depthBits, pixelFormat.stencilBits);
            return true;
        }
    }

    /* No suitable pixel format found */
    return false;
}

void MacOSGLContext::CreateNSGLContext(MacOSGLContext* sharedContext)
{
    /* Get shared NS-OpenGL context */
    auto sharedNSGLCtx = (sharedContext != nullptr ? sharedContext->ctx_ : nullptr);

    /* Create new NS-OpenGL context */
    ctx_ = [[NSOpenGLContext alloc] initWithFormat:pixelFormat_ shareContext:sharedNSGLCtx];

    LLGL_ASSERT(ctx_, "failed to create NSOpenGLContext")

    /* Make new context current */
    MacOSGLContext::MakeNSOpenGLContextCurrent(ctx_);
}

void MacOSGLContext::DeleteNSGLContext()
{
    [pixelFormat_ release];
    [ctx_ makeCurrentContext];
    [ctx_ clearDrawable];
    [ctx_ release];
}


} // /namespace LLGL



// ================================================================================
