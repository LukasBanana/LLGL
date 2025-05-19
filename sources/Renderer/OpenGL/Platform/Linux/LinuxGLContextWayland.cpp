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

    /* Create intermediate GL context OpenGL context with Wayland lib */
    EGLContext intermediateGlc = CreateEGLContextCompatibilityProfile(nullptr, &config_);
    if (intermediateGlc == EGL_NO_CONTEXT)
        LLGL_TRAP("Failed to create EGL context with compatibility profile");

    if (eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, intermediateGlc) != True)
        Log::Errorf("eglMakeCurrent failed on EGL compatibility profile\n");

    if (profile.contextProfile == OpenGLContextProfile::CoreProfile)
    {
        context_ = CreateEGLContextCoreProfile(glcShared, profile.majorVersion, profile.minorVersion, pixelFormat.depthBits, pixelFormat.stencilBits, &config_);
    }

    if (context_)
    {
        if (eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, intermediateGlc) != True)
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

    /* Create core profile */
    const int fbAttribs[] =
    {
        EGL_SURFACE_TYPE,      EGL_WINDOW_BIT,
        EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
        EGL_RENDERABLE_TYPE,   EGL_OPENGL_BIT,
        EGL_RED_SIZE,          8,
        EGL_GREEN_SIZE,        8,
        EGL_BLUE_SIZE,         8,
        EGL_ALPHA_SIZE,        8,
        EGL_DEPTH_SIZE,        depthBits,
        EGL_STENCIL_SIZE,      stencilBits,
        EGL_NONE
    };

    EGLint numConfigs = 0;
    
    if ((eglGetConfigs(display_, nullptr, 0, &numConfigs) != EGL_TRUE) || (numConfigs == 0))
        LLGL_TRAP("Failed to get EGL configs");

    if ((eglChooseConfig(display_, fbAttribs, config, 1, &numConfigs) != EGL_TRUE) || (numConfigs != 1))
        LLGL_TRAP("Failed to choose EGL config");

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
    const int fbAttribs[] =
    {
        EGL_SURFACE_TYPE,      EGL_WINDOW_BIT,
        EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
        EGL_RENDERABLE_TYPE,   EGL_OPENGL_BIT,
        EGL_RED_SIZE,          8,
        EGL_GREEN_SIZE,        8,
        EGL_BLUE_SIZE,         8,
        EGL_ALPHA_SIZE,        8,
        EGL_NONE
    };

    EGLint numConfigs = 0;
    if (eglGetConfigs(display_, nullptr, 0, &numConfigs) != EGL_TRUE || numConfigs == 0)
        LLGL_TRAP("Failed to get EGL configs");

    if ((eglChooseConfig(display_, fbAttribs, config, 1, &numConfigs) != EGL_TRUE) || numConfigs != 1)
        LLGL_TRAP("Failed to choose EGL config");

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
