#if LLGL_LINUX_ENABLE_WAYLAND

#include "LinuxGLContext.h"
#include <LLGL/Backend/OpenGL/NativeHandle.h>
#include <LLGL/Log.h>

namespace LLGL
{

void LinuxGLContext::CreateEGLContext(
    const GLPixelFormat&                pixelFormat,
    const RendererConfigurationOpenGL&  profile,
    const NativeHandle&                 nativeHandle,
    LinuxGLContext*                     sharedContext)
{
    LLGL_ASSERT(nativeHandle.type == NativeHandleType::Wayland, "Window native handle type must be Wayland");

    LLGL_ASSERT_PTR(nativeHandle.wayland.display);
    LLGL_ASSERT_PTR(nativeHandle.wayland.window);

    api_.type = LinuxGLAPIType::EGL;

    EGLContext glcShared = sharedContext != nullptr ? sharedContext->api_.egl.context : nullptr;

    EGLDisplay display = eglGetDisplay(nativeHandle.wayland.display);
    if (display == EGL_NO_DISPLAY)
        LLGL_TRAP("Failed to get EGL display");

    api_.egl.display = display;

    if (eglInitialize(display, nullptr, nullptr) != EGL_TRUE)
        LLGL_TRAP("Failed to initialize EGL");

    /* Create intermediate GL context OpenGL context with Wayland lib */
    EGLContext intermediateGlc = CreateEGLContextCompatibilityProfile(nullptr, &api_.egl.config);
    if (intermediateGlc == EGL_NO_CONTEXT)
        LLGL_TRAP("Failed to create EGL context with compatibility profile");

    if (eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, intermediateGlc) != True)
        Log::Errorf("eglMakeCurrent failed on EGL compatibility profile\n");

    if (profile.contextProfile == OpenGLContextProfile::CoreProfile)
    {
        api_.egl.context = CreateEGLContextCoreProfile(glcShared, profile.majorVersion, profile.minorVersion, pixelFormat.depthBits, pixelFormat.stencilBits, &api_.egl.config);
    }

    if (api_.egl.context)
    {
        if (eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, intermediateGlc) != True)
            Log::Errorf("eglMakeCurrent failed on EGL core profile\n");

        /* Valid core profile created, so we can delete the intermediate GLX context */
        eglDestroyContext(display, intermediateGlc);

        /* Deduce color and depth-stencil formats */
        SetDefaultColorFormat();
        DeduceDepthStencilFormat(pixelFormat.depthBits, pixelFormat.stencilBits);
    }
    else
    {
        /* No core profile created, so we use the intermediate GLX context */
        api_.egl.context = intermediateGlc;

        /* Set fixed color and depth-stencil formats as default values */
        SetDefaultColorFormat();
        SetDefaultDepthStencilFormat();
    }
}

EGLContext LinuxGLContext::CreateEGLContextCoreProfile(EGLContext glcShared, int major, int minor, int depthBits, int stencilBits, EGLConfig* config) {
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

    EGLDisplay display = api_.egl.display;

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
    
    if ((eglGetConfigs(display, nullptr, 0, &numConfigs) != EGL_TRUE) || (numConfigs == 0))
        LLGL_TRAP("Failed to get EGL configs");

    if ((eglChooseConfig(display, fbAttribs, config, 1, &numConfigs) != EGL_TRUE) || (numConfigs != 1))
        LLGL_TRAP("Failed to choose EGL config");

    EGLint contextAttribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, major,
        EGL_CONTEXT_MINOR_VERSION, minor,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_NONE
    };

    eglBindAPI(EGL_OPENGL_API);

    return eglCreateContext(display, *config, glcShared, contextAttribs);
}

EGLContext LinuxGLContext::CreateEGLContextCompatibilityProfile(EGLContext glcShared, EGLConfig* config) {
    EGLDisplay display = api_.egl.display;

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
    if (eglGetConfigs(display, nullptr, 0, &numConfigs) != EGL_TRUE || numConfigs == 0)
        LLGL_TRAP("Failed to get EGL configs");

    if ((eglChooseConfig(display, fbAttribs, config, 1, &numConfigs) != EGL_TRUE) || numConfigs != 1)
        LLGL_TRAP("Failed to choose EGL config");

    EGLint contextAttribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT,
        EGL_NONE
    };

    eglBindAPI(EGL_OPENGL_API);

    return eglCreateContext(display, *config, glcShared, contextAttribs);
}

void LinuxGLContext::DeleteEGLContext() {
    eglMakeCurrent(api_.egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE, nullptr);
    eglDestroyContext(api_.egl.display, api_.egl.context);
}

void LinuxGLContext::CreateProxyEGLContext(
    const GLPixelFormat&                    pixelFormat,
    const NativeHandle&                     nativeWindowHandle,
    const OpenGL::RenderSystemNativeHandle& nativeContextHandle)
{
    LLGL_ASSERT(nativeWindowHandle.type == NativeHandleType::Wayland);
    LLGL_TRAP("TODO");
}

}  // /namespace LLGL

#endif // LLGL_LINUX_ENABLE_WAYLAND

// ================================================================================
