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


std::unique_ptr<GLContext> GLContext::Create(RenderContextDescriptor& desc, Window& window, GLContext* sharedContext)
{
    MacOSGLContext* sharedContextGLNS = (sharedContext != nullptr ? LLGL_CAST(MacOSGLContext*, sharedContext) : nullptr);
    return MakeUnique<MacOSGLContext>(desc, window, sharedContextGLNS);
}

MacOSGLContext::MacOSGLContext(RenderContextDescriptor& desc, Window& window, MacOSGLContext* sharedContext) :
    LLGL::GLContext( sharedContext )
{
    CreatePixelFormat(desc);
    
    NativeHandle nativeHandle;
    window.GetNativeHandle(&nativeHandle);
    
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


/*
 * ======= Private: =======
 */

bool MacOSGLContext::Activate(bool activate)
{
    [ctx_ makeCurrentContext];
    return true;
}

void MacOSGLContext::CreatePixelFormat(const RenderContextDescriptor& desc)
{
    NSOpenGLPixelFormatAttribute attribs[] =
    {
        NSOpenGLPFANoRecovery,
        NSOpenGLPFAAccelerated,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFADepthSize,       32,
        NSOpenGLPFAColorSize,       24,
        NSOpenGLPFAAlphaSize,       8,
        NSOpenGLPFASampleBuffers,   1,
        NSOpenGLPFASamples,         (desc.multiSampling.enabled ? std::max(1u, desc.multiSampling.samples) : 1),
        NSOpenGLPFAStencilSize,     1,
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
    
    /* Create NS-OpenGL context */
    ctx_ = [[NSOpenGLContext alloc] initWithFormat:pixelFormat_ shareContext:sharedNSGLCtx];
    if (!ctx_)
        throw std::runtime_error("failed to create NSOpenGLContext");
}

void MacOSGLContext::DeleteNSGLContext()
{
    [ctx_ makeCurrentContext];
    [ctx_ clearDrawable];
    [ctx_ release];
}


} // /namespace LLGL



// ================================================================================
