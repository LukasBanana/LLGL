/*
 * AndroidGLContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "AndroidGLContext.h"
#include "../../../CheckedCast.h"
#include "../../../../Core/Helper.h"
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/RendererConfiguration.h>


namespace LLGL
{


/*
 * GLContext class
 */

std::unique_ptr<GLContext> GLContext::Create(
    const RenderContextDescriptor&      desc,
    const RendererConfigurationOpenGL&  config,
    Surface&                            surface,
    GLContext*                          sharedContext)
{
    AndroidGLContext* sharedContextEGL = (sharedContext != nullptr ? LLGL_CAST(AndroidGLContext*, sharedContext) : nullptr);
    return MakeUnique<AndroidGLContext>(desc, config, surface, sharedContextEGL);
}


/*
 * LinuxGLContext class
 */

AndroidGLContext::AndroidGLContext(
    const RenderContextDescriptor&      desc,
    const RendererConfigurationOpenGL&  config,
    Surface&                            surface,
    AndroidGLContext*                   sharedContext)
:
    display_  { eglGetDisplay(EGL_DEFAULT_DISPLAY) },
    GLContext { sharedContext                      }
{
    /*NativeHandle nativeHandle = {};
    surface.GetNativeHandle(&nativeHandle, sizeof(nativeHandle));
    CreateContext(desc, config, nativeHandle, sharedContext);*/
    SetSwapInterval(desc.vsync.enabled ? desc.vsync.interval : 0);
}

AndroidGLContext::~AndroidGLContext()
{
    DeleteContext();
}

bool AndroidGLContext::SetSwapInterval(int interval)
{
    return (eglSwapInterval(display_, interval) == EGL_TRUE);
}

bool AndroidGLContext::SwapBuffers()
{
    eglSwapBuffers(display_, surface_);
    return true;
}

void AndroidGLContext::Resize(const Extent2D& resolution)
{
    // dummy
}

std::uint32_t AndroidGLContext::GetSamples() const
{
    return samples_;
}


/*
 * ======= Private: =======
 */

bool AndroidGLContext::Activate(bool activate)
{
    if (activate)
        return eglMakeCurrent(display_, surface_, surface_, context_);
    else
        return eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

void AndroidGLContext::CreateContext(
    const RenderContextDescriptor&      contextDesc,
    const RendererConfigurationOpenGL&  config,
    const NativeHandle&                 nativeHandle,
    AndroidGLContext*                   sharedContext)
{
    EGLContext sharedEGLContext = (sharedContext != nullptr ? sharedContext->context_ : EGL_NO_CONTEXT);
    samples_ = contextDesc.samples;

    /* Initialize EGL display connection (ignore major/minor output parameters) */
    eglInitialize(display_, nullptr, nullptr);
    
    /* Look for a framebuffer configuration; reduce samples if necessary */
    while (samples_ > 1)
    {
        /* Initialize framebuffer configuration */
        const EGLint attribs[] =
        {
            EGL_RED_SIZE,       8,
            EGL_GREEN_SIZE,     8,
            EGL_BLUE_SIZE,      8,
            //EGL_ALPHA_SIZE,     8,
            EGL_DEPTH_SIZE,     contextDesc.videoMode.depthBits,
            EGL_STENCIL_SIZE,   contextDesc.videoMode.stencilBits,
            EGL_SAMPLE_BUFFERS, (samples_ > 1 ? 1 : 0),
            EGL_SAMPLES,        (samples_ > 1 ? static_cast<EGLint>(samples_) : 1),
            EGL_NONE
        };
        
        /* Choose configuration */
        EGLint numConfigs = 0;
        EGLBoolean success = eglChooseConfig(display_, attribs, &config_, 1, &numConfigs);
        
        /* Reduce number of sample if configuration failed */
        if (success == EGL_TRUE && numConfigs >= 1)
            break;
        else
            --samples_;
    }
    
    /* Create EGL context */
    EGLint major = 3, minor = 0;
    
    if (!(config.majorVersion == 0 && config.minorVersion == 0))
    {
        major = config.majorVersion;
        minor = config.minorVersion;
    }
    
    const EGLint contextAttribs[] =
    {
        EGL_CONTEXT_MAJOR_VERSION,          major,
        EGL_CONTEXT_MINOR_VERSION,          minor,
        #ifdef LLGL_DEBUG
        EGL_CONTEXT_OPENGL_DEBUG,           EGL_TRUE,
        EGL_CONTEXT_OPENGL_ROBUST_ACCESS,   EGL_TRUE,
        #endif
        EGL_NONE
    };
    
    context_ = eglCreateContext(display_, config_, sharedEGLContext, contextAttribs);
    if (!context_)
        throw std::runtime_error("failed to create EGL context");
    
    /* Create drawable surface */
    surface_ = eglCreateWindowSurface(display_, config_, nativeHandle.window, nullptr);
    if (!surface_)
        throw std::runtime_error("failed to create EGL surface");

    /* Make context current */
    eglMakeCurrent(display_, surface_, surface_, context_);
}

void AndroidGLContext::DeleteContext()
{
    eglDestroyContext(display_, context_);
}


} // /namespace LLGL



// ================================================================================
