/*
 * MacOSGLContext.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MacOSGLContext.h"
#include "../../../../Platform/MacOS/MacOSWindow.h"
#include "../../../CheckedCast.h"
#include "../../../../Core/Helper.h"
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Log.h>


namespace LLGL
{


std::unique_ptr<GLContext> GLContext::Create(RenderContextDescriptor& desc, Surface& surface, GLContext* sharedContext)
{
    MacOSGLContext* sharedContextGLNS = (sharedContext != nullptr ? LLGL_CAST(MacOSGLContext*, sharedContext) : nullptr);
    return MakeUnique<MacOSGLContext>(desc, surface, sharedContextGLNS);
}

MacOSGLContext::MacOSGLContext(RenderContextDescriptor& desc, Surface& surface, MacOSGLContext* sharedContext) :
    LLGL::GLContext { sharedContext }
{
    CreatePixelFormat(desc);
    
    NativeHandle nativeHandle;
    surface.GetNativeHandle(&nativeHandle);
    
    CreateNSGLContext(nativeHandle, sharedContext);
}

MacOSGLContext::~MacOSGLContext()
{
    DeleteNSGLContext();
}

bool MacOSGLContext::SetSwapInterval(int interval)
{
    [ctx_ setValues:&interval forParameter:NSOpenGLCPSwapInterval];
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


/*
 * ======= Private: =======
 */

bool MacOSGLContext::Activate(bool activate)
{
    [ctx_ makeCurrentContext];
    [ctx_ setView:[wnd_ contentView]];
    return true;
}

static NSOpenGLPixelFormatAttribute TranslateNSOpenGLProfile(const ProfileOpenGLDescriptor& profile)
{
    if (profile.contextProfile == OpenGLContextProfile::CompatibilityProfile)
        return NSOpenGLProfileVersionLegacy;
    if (profile.contextProfile == OpenGLContextProfile::CoreProfile)
    {
        if (profile.majorVersion < 0 || profile.minorVersion < 0 || (profile.majorVersion == 4 && profile.minorVersion == 1))
            return NSOpenGLProfileVersion4_1Core;
        if (profile.majorVersion == 3 && profile.minorVersion == 2)
            return NSOpenGLProfileVersion3_2Core;
    }
    throw std::runtime_error("failed to choose OpenGL profile (only compatibility profile, 3.2 core profile, and 4.1 core profile are supported)");
}

void MacOSGLContext::CreatePixelFormat(const RenderContextDescriptor& desc)
{
    NSOpenGLPixelFormatAttribute attribs[] =
    {
        //NSOpenGLPFANoRecovery,
        NSOpenGLPFAAccelerated,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAOpenGLProfile,   TranslateNSOpenGLProfile(desc.profileOpenGL),
        NSOpenGLPFADepthSize,       static_cast<std::uint32_t>(desc.videoMode.depthBits),
        NSOpenGLPFAStencilSize,     static_cast<std::uint32_t>(desc.videoMode.stencilBits),
        NSOpenGLPFAColorSize,       24,
        NSOpenGLPFAAlphaSize,       8,
        NSOpenGLPFASampleBuffers,   (desc.multiSampling.enabled ? 1u : 0u),
        NSOpenGLPFASamples,         desc.multiSampling.SampleCount(),
        //NSOpenGLPFAFullScreen,
        0
    };
    
    while (true)
    {
        /* Allocate NS-OpenGL pixel format */
        pixelFormat_ = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
        if (pixelFormat_)
            break;
        
        /* Find best suitable pixel format */
        //todo...
        
        throw std::runtime_error("failed to find suitable OpenGL pixel format");
    }
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
