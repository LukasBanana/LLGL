/*
 * AndroidGLContext.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "AndroidGLContext.h"
#include "../../../CheckedCast.h"
#include "../../../../Core/CoreUtils.h"
#include <LLGL/RendererConfiguration.h>


namespace LLGL
{


/*
 * GLContext class
 */

std::unique_ptr<GLContext> GLContext::Create(
    const GLPixelFormat&                pixelFormat,
    const RendererConfigurationOpenGL&  profile,
    Surface&                            surface,
    GLContext*                          sharedContext)
{
    AndroidGLContext* sharedContextEGL = (sharedContext != nullptr ? LLGL_CAST(AndroidGLContext*, sharedContext) : nullptr);
    return MakeUnique<AndroidGLContext>(pixelFormat, profile, surface, sharedContextEGL);
}


/*
 * LinuxGLContext class
 */

AndroidGLContext::AndroidGLContext(
    const GLPixelFormat&                pixelFormat,
    const RendererConfigurationOpenGL&  profile,
    Surface&                            surface,
    AndroidGLContext*                   sharedContext)
:
    display_ { eglGetDisplay(EGL_DEFAULT_DISPLAY) }
{
    CreateContext(pixelFormat, profile, sharedContext);
}

AndroidGLContext::~AndroidGLContext()
{
    DeleteContext();
}

void AndroidGLContext::Resize(const Extent2D& resolution)
{
    // dummy
}

int AndroidGLContext::GetSamples() const
{
    return samples_;
}


/*
 * ======= Private: =======
 */

bool AndroidGLContext::SetSwapInterval(int interval)
{
    return (eglSwapInterval(display_, interval) == EGL_TRUE);
}

bool AndroidGLContext::SelectConfig(const GLPixelFormat& pixelFormat)
{
    /* Look for a framebuffer configuration; reduce samples if necessary */
    for (samples_ = pixelFormat.samples; samples_ > 1; --samples_)
    {
        /* Initialize framebuffer configuration */
        const EGLint attribs[] =
        {
            EGL_RED_SIZE,       8,
            EGL_GREEN_SIZE,     8,
            EGL_BLUE_SIZE,      8,
            //EGL_ALPHA_SIZE,     8,
            EGL_DEPTH_SIZE,     pixelFormat.depthBits,
            EGL_STENCIL_SIZE,   pixelFormat.stencilBits,
            EGL_SAMPLE_BUFFERS, (samples_ > 1 ? 1 : 0),
            EGL_SAMPLES,        (samples_ > 1 ? samples_ : 1),
            EGL_NONE
        };

        /* Choose configuration */
        EGLint numConfigs = 0;
        EGLBoolean success = eglChooseConfig(display_, attribs, &config_, 1, &numConfigs);

        /* Reduce number of sample if configuration failed */
        if (success == EGL_TRUE && numConfigs >= 1)
        {
            SetDefaultColorFormat();
            DeduceDepthStencilFormat(pixelFormat.depthBits, pixelFormat.stencilBits);
            return true;
        }
    }

    /* No suitable configuration found */
    return false;
}

void AndroidGLContext::CreateContext(
    const GLPixelFormat&                pixelFormat,
    const RendererConfigurationOpenGL&  profile,
    AndroidGLContext*                   sharedContext)
{
    /* Initialize EGL display connection (ignore major/minor output parameters) */
    if (!eglInitialize(display_, nullptr, nullptr))
        throw std::runtime_error("eglInitialize failed");

    /* Select EGL context configuration for pixel format */
    if (!SelectConfig(pixelFormat))
        throw std::runtime_error("eglChooseConfig failed");

    /* Set up EGL profile attributes */
    EGLint major = 3, minor = 0;

    if (!(profile.majorVersion == 0 && profile.minorVersion == 0))
    {
        major = profile.majorVersion;
        minor = profile.minorVersion;
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

    /* Create EGL context with optional shared EGL context */
    EGLContext sharedEGLContext = (sharedContext != nullptr ? sharedContext->context_ : EGL_NO_CONTEXT);
    context_ = eglCreateContext(display_, config_, sharedEGLContext, contextAttribs);
    if (!context_)
        throw std::runtime_error("eglCreateContext failed");
}

void AndroidGLContext::DeleteContext()
{
    eglDestroyContext(display_, context_);
}


} // /namespace LLGL



// ================================================================================
