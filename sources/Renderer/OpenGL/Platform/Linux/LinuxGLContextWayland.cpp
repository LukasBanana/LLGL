/*
 * LinuxGLContextWayland.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */
#if LLGL_LINUX_ENABLE_WAYLAND

#include "LinuxGLContextWayland.h"
#include <LLGL/Backend/OpenGL/NativeHandle.h>
#include <LLGL/Log.h>
#include <wayland-egl.h>

#include "LinuxGLCore.h"
#include "LinuxSharedEGLSurface.h"

namespace LLGL
{


void LinuxGLContextWayland::CreateEGLContext(
    const GLPixelFormat&                pixelFormat,
    const RendererConfigurationOpenGL&  profile,
    const NativeHandle&                 nativeHandle,
    LinuxGLContextWayland*              sharedContext)
{
    LLGL_ASSERT(nativeHandle.type == NativeType::Wayland, "Window native handle type must be Wayland");

    LLGL_ASSERT_PTR(nativeHandle.wayland.display);
    LLGL_ASSERT_PTR(nativeHandle.wayland.window);

    EGLContext glcShared = sharedContext != nullptr ? sharedContext->context_ : nullptr;

    EGLDisplay display = eglGetDisplay(nativeHandle.wayland.display);
    if (display == EGL_NO_DISPLAY)
        LLGL_TRAP("Failed to get EGL display");

    display_ = display;

    if (eglInitialize(display, nullptr, nullptr) != EGL_TRUE)
        LLGL_TRAP("Failed to initialize EGL");

    /* Select EGL context configuration for pixel format */
    if (!SelectConfig(pixelFormat))
    {
        LLGL_TRAP(
            "eglChooseConfig [colorBits = %d, depthBits = %d, stencilBits = %d, samples = %d] failed (%s)",
            pixelFormat.colorBits, pixelFormat.depthBits, pixelFormat.stencilBits, pixelFormat.samples,
            EGLErrorToString()
        );
    }

    /* Create intermediate GL context OpenGL context with Wayland lib */
    EGLContext intermediateGlc = CreateEGLContextCompatibilityProfile(nullptr, &config_);
    if (intermediateGlc == EGL_NO_CONTEXT)
        LLGL_TRAP("Failed to create EGL context with compatibility profile", EGLErrorToString());

    if (eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, intermediateGlc) != True)
        Log::Errorf("eglMakeCurrent failed on EGL compatibility profile\n");

    if (profile.contextProfile == OpenGLContextProfile::CoreProfile)
    {
        context_ = CreateEGLContextCoreProfile(glcShared, profile.majorVersion, profile.minorVersion, pixelFormat.depthBits, pixelFormat.stencilBits, &config_);
    }

    if (sharedContext != nullptr)
    {
        /* Share EGLSurface with shared context */
        sharedSurface_ = sharedContext->GetSharedEGLSurface();
    }
    else
    {
        sharedSurface_ = std::make_shared<LinuxSharedEGLSurface>(display_, config_, nullptr);
    }

    if (context_)
    {
        EGLSurface nativeSurface = sharedSurface_->GetEGLSurface();
        if (eglMakeCurrent(display, nativeSurface, nativeSurface, intermediateGlc) != True)
            Log::Errorf("eglMakeCurrent failed on EGL core profile\n");

        /* Valid core profile created, so we can delete the intermediate EGL context */
        eglDestroyContext(display, intermediateGlc);

        /* Deduce color and depth-stencil formats */
        SetDefaultColorFormat();
        DeduceDepthStencilFormat(pixelFormat.depthBits, pixelFormat.stencilBits);
    }
    else
    {
        /* No core profile created, so we use the intermediate EGL context */
        context_ = intermediateGlc;

        /* Set fixed color and depth-stencil formats as default values */
        SetDefaultColorFormat();
        SetDefaultDepthStencilFormat();
    }
}

int LinuxGLContextWayland::GetSamples() const
{
    return samples_;
}

bool LinuxGLContextWayland::SelectConfig(const GLPixelFormat& pixelFormat)
{
    /* Look for a framebuffer configuration; reduce samples if necessary */
    for (samples_ = std::max(1, pixelFormat.samples); samples_ > 0; --samples_)
    {
        /* Initialize framebuffer configuration */
        EGLint attribs[] =
        {
            EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
            EGL_RED_SIZE,       8,
            EGL_GREEN_SIZE,     8,
            EGL_BLUE_SIZE,      8,
            EGL_ALPHA_SIZE,     (pixelFormat.colorBits == 32 ? 8 : 0),
            EGL_DEPTH_SIZE,     pixelFormat.depthBits,
            EGL_STENCIL_SIZE,   pixelFormat.stencilBits,
            EGL_SAMPLE_BUFFERS, 1,
            EGL_SAMPLES,        samples_,
            EGL_NONE
        };

        if (samples_ <= 1)
        {
            /* Cut off EGL_SAMPLE* entries in case EGL context doesn't support them at all */
            constexpr int sampleBuffersArrayIndex = 14;
            LLGL_ASSERT(attribs[sampleBuffersArrayIndex] == EGL_SAMPLE_BUFFERS);
            attribs[sampleBuffersArrayIndex] = EGL_NONE;
        }

        /* Choose configuration */
        EGLint numConfigs = 0;
        EGLBoolean success = eglChooseConfig(display_, attribs, &config_, 1, &numConfigs);

        /* Reduce number of sample if configuration failed */
        if (success == EGL_TRUE && numConfigs > 0)
        {
            SetDefaultColorFormat();
            DeduceDepthStencilFormat(pixelFormat.depthBits, pixelFormat.stencilBits);
            return true;
        }
    }

    /* No suitable configuration found */
    return false;
}

EGLContext LinuxGLContextWayland::CreateEGLContextCoreProfile(EGLContext glcShared, int major, int minor, int depthBits, int stencilBits, EGLConfig* config)
{
    /* Query supported GL versions */
    if (major == 0 && minor == 0)
    {
        /* Query highest possible GL version from intermediate context */
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
    }

    if (major < 3)
    {
        /* Don't try to create a core profile when GL version is below 3.0 */
        Log::Errorf("cannot create OpenGL core profile with GL version %d.%d\n", major, minor);
        return nullptr;
    }

    EGLint contextAttribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, major,
        EGL_CONTEXT_MINOR_VERSION, minor,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_NONE
    };

    eglBindAPI(EGL_OPENGL_API);

    return eglCreateContext(display_, *config, glcShared, contextAttribs);
}

EGLContext LinuxGLContextWayland::CreateEGLContextCompatibilityProfile(EGLContext glcShared, EGLConfig* config)
{
    EGLint contextAttribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT,
        EGL_NONE
    };

    eglBindAPI(EGL_OPENGL_API);

    return eglCreateContext(display_, *config, glcShared, contextAttribs);
}

void LinuxGLContextWayland::DeleteEGLContext()
{
    eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, nullptr);
    eglDestroyContext(display_, context_);
}

void LinuxGLContextWayland::CreateProxyEGLContext(
    const GLPixelFormat&                    pixelFormat,
    const NativeHandle&                     nativeWindowHandle,
    const OpenGL::RenderSystemNativeHandle& nativeContextHandle)
{
    LLGL_ASSERT(nativeWindowHandle.type == NativeType::Wayland);
    LLGL_TRAP_NOT_IMPLEMENTED("Wayland proxy EGL context");
}


/*
 * LinuxGLContext class
 */

LinuxGLContextWayland::LinuxGLContextWayland(
    const GLPixelFormat&                    pixelFormat,
    const RendererConfigurationOpenGL&      profile,
    Surface&                                surface,
    LinuxGLContextWayland*                  sharedContext,
    const OpenGL::RenderSystemNativeHandle* customNativeHandle)
:
    samples_ { pixelFormat.samples }
{
    /* Create EGL or proxy context if a custom one is specified */
    NativeHandle nativeWindowHandle = {};
    surface.GetNativeHandle(&nativeWindowHandle, sizeof(nativeWindowHandle));

    if (customNativeHandle != nullptr)
    {
        isProxyGLC_ = true;
        CreateProxyEGLContext(pixelFormat, nativeWindowHandle, *customNativeHandle);
    }
    else
    {
        CreateEGLContext(pixelFormat, profile, nativeWindowHandle, sharedContext);
    }
}

LinuxGLContextWayland::~LinuxGLContextWayland()
{
    if (!isProxyGLC_)
        DeleteEGLContext();
}

bool LinuxGLContextWayland::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(OpenGL::RenderSystemNativeHandle))
    {
        auto* nativeHandleGL = static_cast<OpenGL::RenderSystemNativeHandle*>(nativeHandle);

        nativeHandleGL->egl = context_;
        nativeHandleGL->type = OpenGL::RenderSystemNativeType::EGL;

        return true;
    }
    return false;
}

OpenGL::RenderSystemNativeType LinuxGLContextWayland::GetNativeType() const
{
    return OpenGL::RenderSystemNativeType::EGL;
}


/*
 * ======= Private: =======
 */

bool LinuxGLContextWayland::SetSwapInterval(int interval)
{
    return (eglSwapInterval(display_, interval) == EGL_TRUE);
}


}  // /namespace LLGL

#endif // LLGL_LINUX_ENABLE_WAYLAND



// ================================================================================
