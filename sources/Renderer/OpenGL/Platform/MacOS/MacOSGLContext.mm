/*
 * MacOSGLContext.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MacOSGLContext.h"
#include "../../../../Platform/MacOS/MacOSWindow.h"
#include "../../../../Core/Helper.h"
#include "../../../CheckedCast.h"
#include "../../../TextureUtils.h"
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Log.h>


namespace LLGL
{


std::unique_ptr<GLContext> GLContext::Create(
    const RenderContextDescriptor&      desc,
    const RendererConfigurationOpenGL&  config,
    Surface&                            surface,
    GLContext*                          sharedContext)
{
    MacOSGLContext* sharedContextGLNS = (sharedContext != nullptr ? LLGL_CAST(MacOSGLContext*, sharedContext) : nullptr);
    return MakeUnique<MacOSGLContext>(desc, config, surface, sharedContextGLNS);
}

MacOSGLContext::MacOSGLContext(
    const RenderContextDescriptor&      desc,
    const RendererConfigurationOpenGL&  config,
    Surface&                            surface,
    MacOSGLContext*                     sharedContext)
:
    LLGL::GLContext { sharedContext }
{
    if (!CreatePixelFormat(desc, config))
        throw std::runtime_error("failed to find suitable OpenGL pixel format");

    NativeHandle nativeHandle = {};
    surface.GetNativeHandle(&nativeHandle, sizeof(nativeHandle));

    CreateNSGLContext(nativeHandle, sharedContext);
}

MacOSGLContext::~MacOSGLContext()
{
    DeleteNSGLContext();
}

bool MacOSGLContext::SetSwapInterval(int interval)
{
    #if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_14
    [ctx_ setValues:&interval forParameter:NSOpenGLContextParameterSwapInterval];
    #else
    [ctx_ setValues:&interval forParameter:NSOpenGLCPSwapInterval];
    #endif
    return true;
}

bool MacOSGLContext::SwapBuffers()
{
    [ctx_ flushBuffer];
    return true;
}

void MacOSGLContext::Resize(const Extent2D& resolution)
{
    [ctx_ update];
}

std::uint32_t MacOSGLContext::GetSamples() const
{
    return samples_;
}


/*
 * ======= Private: =======
 */

bool MacOSGLContext::Activate(bool activate)
{
    /* Make context current */
    [ctx_ makeCurrentContext];

    /* 'setView' is deprecated since macOS 10.14 together with OpenGL in general, so suppress this deprecation warning */
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"

    [ctx_ setView:[wnd_ contentView]];

    #pragma clang diagnostic pop

    [ctx_ update];

    return true;
}

static NSOpenGLPixelFormatAttribute TranslateNSOpenGLProfile(const RendererConfigurationOpenGL& config)
{
    if (config.contextProfile == OpenGLContextProfile::CompatibilityProfile)
    {
        /* Choose OpenGL compatibility profile */
        return NSOpenGLProfileVersionLegacy;
    }
    if (config.contextProfile == OpenGLContextProfile::CoreProfile)
    {
        if ((config.majorVersion == 0 && config.minorVersion == 0) ||
            (config.majorVersion == 4 && config.minorVersion == 1))
        {
            /* Choose OpenGL 4.1 core profile (default) */
            return NSOpenGLProfileVersion4_1Core;
        }
        if (config.majorVersion == 3 && config.minorVersion == 2)
        {
            /* Choose OpenGL 3.2 core profile */
            return NSOpenGLProfileVersion3_2Core;
        }
    }
    throw std::runtime_error("failed to choose OpenGL profile (only compatibility profile, 3.2 core profile, and 4.1 core profile are supported)");
}

bool MacOSGLContext::CreatePixelFormat(const RenderContextDescriptor& desc, const RendererConfigurationOpenGL& config)
{
    /* Find suitable pixel format (for samples > 0) */
    for (samples_ = GetClampedSamples(desc.samples); samples_ > 0; --samples_)
    {
        NSOpenGLPixelFormatAttribute attribs[] =
        {
            NSOpenGLPFAAccelerated,
            NSOpenGLPFADoubleBuffer,
            NSOpenGLPFAOpenGLProfile,   TranslateNSOpenGLProfile(config),
            NSOpenGLPFADepthSize,       static_cast<std::uint32_t>(desc.videoMode.depthBits),
            NSOpenGLPFAStencilSize,     static_cast<std::uint32_t>(desc.videoMode.stencilBits),
            NSOpenGLPFAColorSize,       24,
            NSOpenGLPFAAlphaSize,       8,
            //NSOpenGLPFAMultisample,
            NSOpenGLPFASampleBuffers,   (desc.samples > 1 ? 1u : 0u),
            NSOpenGLPFASamples,         samples_,
            0
        };

        /* Allocate NS-OpenGL pixel format */
        pixelFormat_ = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
        if (pixelFormat_)
            return true;
    }
    return false;
}

void MacOSGLContext::CreateNSGLContext(const NativeHandle& nativeHandle, MacOSGLContext* sharedContext)
{
    /* Get shared NS-OpenGL context */
    auto sharedNSGLCtx = (sharedContext != nullptr ? sharedContext->ctx_ : nullptr);

    if (sharedNSGLCtx/* && if not create custom GL context */)
    {
        /* Share this NS-OpenGL context */
        ctx_ = sharedNSGLCtx;
    }
    else
    {
        /* Create new NS-OpenGL context */
        ctx_ = [[NSOpenGLContext alloc] initWithFormat:pixelFormat_ shareContext:sharedNSGLCtx];
        if (!ctx_)
            throw std::runtime_error("failed to create NSOpenGLContext");
    }

    /* Store native window handle */
    wnd_ = nativeHandle.window;

    /* Make current and set view to specified window */
    Activate(true);
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
