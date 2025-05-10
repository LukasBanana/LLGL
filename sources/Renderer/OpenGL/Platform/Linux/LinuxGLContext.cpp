/*
 * LinuxGLContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "LinuxGLContext.h"
#include "../../Ext/GLExtensions.h"
#include "../../Ext/GLExtensionLoader.h"
#include "../../GLCore.h"
#include "../../../CheckedCast.h"
#include "../../../StaticAssertions.h"
#include "../../../RenderSystemUtils.h"
#include "../../../../Core/CoreUtils.h"
#include "../../../../Core/Assertion.h"
#include "../../../../Platform/Linux/LinuxDisplay.h"
#include <LLGL/Backend/OpenGL/NativeHandle.h>
#include <LLGL/Log.h>
#include <algorithm>

#include <wayland-egl.h>

namespace LLGL
{


#ifndef GLX_CONTEXT_MAJOR_VERSION_ARB
#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#endif

#ifndef GLX_CONTEXT_MINOR_VERSION_ARB
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
#endif

typedef GLXContext (*GXLCREATECONTEXTATTRIBARBPROC)(::Display*, GLXFBConfig, GLXContext, Bool, const int*);


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
    LinuxGLContext* sharedContextGLX = (sharedContext != nullptr ? LLGL_CAST(LinuxGLContext*, sharedContext) : nullptr);
    return MakeUnique<LinuxGLContext>(
        pixelFormat, profile, surface, sharedContextGLX,
        GetRendererNativeHandle<OpenGL::RenderSystemNativeHandle>(customNativeHandle)
    );
}


/*
 * LinuxGLContext class
 */

LinuxGLContext::LinuxGLContext(
    const GLPixelFormat&                    pixelFormat,
    const RendererConfigurationOpenGL&      profile,
    Surface&                                surface,
    LinuxGLContext*                         sharedContext,
    const OpenGL::RenderSystemNativeHandle* customNativeHandle)
:
    samples_ { pixelFormat.samples }
{
    /* Notify the shared X11 display that it'll be used by libGL.so to ensure a clean teardown */
    LinuxSharedX11Display::RetainLibGL();

    /* Create GLX or proxy context if a custom one is specified */
    NativeHandle nativeWindowHandle = {};
    surface.GetNativeHandle(&nativeWindowHandle, sizeof(nativeWindowHandle));

    bool is_wayland = nativeWindowHandle.type == NativeHandleType::Wayland;

    if (customNativeHandle != nullptr)
    {
        isProxyGLC_ = true;
        if (is_wayland)
            CreateProxyEGLContext(pixelFormat, nativeWindowHandle, *customNativeHandle);
        else
            CreateProxyGLXContext(pixelFormat, nativeWindowHandle, *customNativeHandle);
    }
    else
    {
        if (is_wayland)
            CreateEGLContext(pixelFormat, profile, nativeWindowHandle, sharedContext);
        else
            CreateGLXContext(pixelFormat, profile, nativeWindowHandle, sharedContext);
    }
}

LinuxGLContext::~LinuxGLContext()
{
    if (!isProxyGLC_) {
        if (api_.type == APIType::EGL) {
            DeleteGLXContext();
        } else if (api_.type == APIType::GLX) {
            DeleteEGLContext();
        }
    }
}

int LinuxGLContext::GetSamples() const
{
    return samples_;
}

bool LinuxGLContext::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(OpenGL::RenderSystemNativeHandle))
    {
        auto* nativeHandleGL = static_cast<OpenGL::RenderSystemNativeHandle*>(nativeHandle);

        if (api_.type == APIType::EGL) {
            nativeHandleGL->egl = api_.egl.context;
            nativeHandleGL->type = OpenGL::RenderSystemNativeHandleType::EGL;
        } else if (api_.type == APIType::GLX) {
            nativeHandleGL->glx = api_.glx.context;
            nativeHandleGL->type = OpenGL::RenderSystemNativeHandleType::GLX;
        }

        return true;
    }
    return false;
}

::XVisualInfo* LinuxGLContext::ChooseVisual(::Display* display, int screen, const GLPixelFormat& pixelFormat, int& outSamples)
{
    GLXFBConfig framebufferConfig = 0;

    /* Find suitable multi-sample format (for samples > 1) */
    for (outSamples = pixelFormat.samples; outSamples > 1; --outSamples)
    {
        /* Create framebuffer configuration for multi-sampling */
        const int framebufferAttribs[] =
        {
            GLX_DOUBLEBUFFER,   True,
            GLX_X_RENDERABLE,   True,
            GLX_DRAWABLE_TYPE,  GLX_WINDOW_BIT,
            GLX_RENDER_TYPE,    GLX_RGBA_BIT,
            GLX_X_VISUAL_TYPE,  GLX_TRUE_COLOR,
            GLX_RED_SIZE,       8,
            GLX_GREEN_SIZE,     8,
            GLX_BLUE_SIZE,      8,
            GLX_ALPHA_SIZE,     (pixelFormat.colorBits == 32 ? 8 : 0),
            GLX_DEPTH_SIZE,     pixelFormat.depthBits,
            GLX_STENCIL_SIZE,   pixelFormat.stencilBits,
            GLX_SAMPLE_BUFFERS, 1,
            GLX_SAMPLES,        outSamples,
            None
        };

        int fbConfigsCount = 0;
        GLXFBConfig* fbConfigs = glXChooseFBConfig(display, screen, framebufferAttribs, &fbConfigsCount);

        if (fbConfigs != nullptr)
        {
            if (fbConfigsCount > 0)
            {
                framebufferConfig = fbConfigs[0];
                if (framebufferConfig != 0)
                    break;
            }
            XFree(fbConfigs);
        }
    }

    if (framebufferConfig)
    {
        /* Choose XVisualInfo from FB config */
        return glXGetVisualFromFBConfig(display, framebufferConfig);
    }
    else
    {
        /* Choose standard XVisualInfo structure */
        int visualAttribs[] =
        {
            GLX_RGBA,
            GLX_DOUBLEBUFFER,
            GLX_RED_SIZE,       8,
            GLX_GREEN_SIZE,     8,
            GLX_BLUE_SIZE,      8,
            GLX_ALPHA_SIZE,     (pixelFormat.colorBits == 32 ? 8 : 0),
            GLX_DEPTH_SIZE,     pixelFormat.depthBits,
            GLX_STENCIL_SIZE,   pixelFormat.stencilBits,
            None
        };

        return glXChooseVisual(display, screen, visualAttribs);
    }
}


/*
 * ======= Private: =======
 */

bool LinuxGLContext::SetSwapInterval(int interval)
{
    /* Load GL extension "GLX_SGI/MESA/EXT_swap_control" to set v-sync interval */
    LoadSwapIntervalProcs();

    if (glXSwapIntervalMESA != nullptr)
    {
        /* Prefer MESA extension since SGI extension returns false for interval 0 */
        return (glXSwapIntervalMESA(static_cast<unsigned int>(interval)) == 0);
    }

    if (glXSwapIntervalEXT != nullptr)
    {
        /* Can only assume this function succeeded as it doesn't return any status */
        ::Display* display = glXGetCurrentDisplay();
        ::GLXDrawable drawable = glXGetCurrentDrawable();
        if (drawable)
        {
            glXSwapIntervalEXT(display, drawable, interval);
            return true;
        }
    }

    if (glXSwapIntervalSGI != nullptr)
    {
        /* Fallback to SGI extension. This is known to *not* support interval=0 */
        return (glXSwapIntervalSGI(interval) == 0);
    }

    return false;
}

void LinuxGLContext::CreateEGLContext(
    const GLPixelFormat&                pixelFormat,
    const RendererConfigurationOpenGL&  profile,
    const NativeHandle&                 nativeHandle,
    LinuxGLContext*                     sharedContext)
{
    LLGL_ASSERT(nativeHandle.type == NativeHandleType::Wayland, "Window native handle type must be Wayland");

    LLGL_ASSERT_PTR(nativeHandle.wayland.display);
    LLGL_ASSERT_PTR(nativeHandle.wayland.window);

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

    if (profile.contextProfile == OpenGLContextProfile::CoreProfile) {
        api_.egl.context = CreateEGLContextCoreProfile(glcShared, profile.majorVersion, profile.minorVersion, pixelFormat.depthBits, pixelFormat.stencilBits, &api_.egl.config);
    }

    if (api_.egl.context) {
        if (eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, intermediateGlc) != True)
            Log::Errorf("eglMakeCurrent failed on EGL core profile\n");

        /* Valid core profile created, so we can delete the intermediate GLX context */
        eglDestroyContext(display, intermediateGlc);

        /* Deduce color and depth-stencil formats */
        SetDefaultColorFormat();
        DeduceDepthStencilFormat(pixelFormat.depthBits, pixelFormat.stencilBits);
    } else {
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


void LinuxGLContext::CreateGLXContext(
    const GLPixelFormat&                pixelFormat,
    const RendererConfigurationOpenGL&  profile,
    const NativeHandle&                 nativeHandle,
    LinuxGLContext*                     sharedContext)
{
    LLGL_ASSERT(nativeHandle.type == NativeHandleType::X11, "Window native handle type must be X11");

    LLGL_ASSERT_PTR(nativeHandle.x11.display);
    LLGL_ASSERT_PTR(nativeHandle.x11.window);

    GLXContext glcShared = sharedContext != nullptr ? sharedContext->api_.glx.context : nullptr;

    ::Display* display = nativeHandle.x11.display;

    /* Get X11 display, window, and visual information */
    api_.glx.display = display;

    /* Ensure GLX is a supported X11 extension */
    int errorBase = 0, eventBase = 0;
    if (glXQueryExtension(display, &errorBase, &eventBase) == False)
        LLGL_TRAP("GLX extension is not supported by X11 implementation");

    /* Get X11 visual information or choose it now */
    ::XVisualInfo* visual = nativeHandle.x11.visual;
    if (visual == nullptr)
    {
        visual = LinuxGLContext::ChooseVisual(display, nativeHandle.x11.screen, pixelFormat, samples_);
        LLGL_ASSERT(visual != nullptr, "failed to choose X11VisualInfo");
    }

    /* Create intermediate GL context OpenGL context with X11 lib */
    GLXContext intermediateGlc = CreateGLXContextCompatibilityProfile(visual, nullptr);

    if (glXMakeCurrent(display, nativeHandle.x11.window, intermediateGlc) != True)
        Log::Errorf("glXMakeCurrent failed on GLX compatibility profile\n");

    if (profile.contextProfile == OpenGLContextProfile::CoreProfile)
    {
        /* Create core profile */
        api_.glx.context = CreateGLXContextCoreProfile(glcShared, profile.majorVersion, profile.minorVersion, pixelFormat.depthBits, pixelFormat.stencilBits);
    }

    if (api_.glx.context)
    {
        /* Make new OpenGL context current */
        if (glXMakeCurrent(display, nativeHandle.x11.window, api_.glx.context) != True)
            Log::Errorf("glXMakeCurrent failed on GLX core profile\n");

        /* Valid core profile created, so we can delete the intermediate GLX context */
        glXDestroyContext(display, intermediateGlc);

        /* Deduce color and depth-stencil formats */
        SetDefaultColorFormat();
        DeduceDepthStencilFormat(pixelFormat.depthBits, pixelFormat.stencilBits);
    }
    else
    {
        /* No core profile created, so we use the intermediate GLX context */
        api_.glx.context = intermediateGlc;

        /* Set fixed color and depth-stencil formats as default values */
        SetDefaultColorFormat();
        SetDefaultDepthStencilFormat();
    }
}

void LinuxGLContext::DeleteGLXContext()
{
    glXMakeCurrent(api_.glx.display, None, nullptr);
    glXDestroyContext(api_.glx.display, api_.glx.context);
}

void LinuxGLContext::DeleteEGLContext() {
    eglMakeCurrent(api_.glx.display, EGL_NO_SURFACE, EGL_NO_SURFACE, nullptr);
    eglDestroyContext(api_.glx.display, api_.glx.context);
}

GLXContext LinuxGLContext::CreateGLXContextCoreProfile(GLXContext glcShared, int major, int minor, int depthBits, int stencilBits)
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

    /* Load GL extension to create core profile */
    GXLCREATECONTEXTATTRIBARBPROC glXCreateContextAttribsARB = nullptr;
    glXCreateContextAttribsARB = (GXLCREATECONTEXTATTRIBARBPROC)glXGetProcAddressARB(reinterpret_cast<const GLubyte*>("glXCreateContextAttribsARB"));

    if (glXCreateContextAttribsARB != nullptr)
    {
        /* Create core profile */
        const int fbAttribs[] =
        {
            GLX_X_RENDERABLE,   True,
            GLX_DOUBLEBUFFER,   True,
            GLX_DRAWABLE_TYPE,  GLX_WINDOW_BIT,
            GLX_RENDER_TYPE,    GLX_RGBA_BIT,
            GLX_X_VISUAL_TYPE,  GLX_TRUE_COLOR,
            GLX_RED_SIZE,       8,
            GLX_GREEN_SIZE,     8,
            GLX_BLUE_SIZE,      8,
            GLX_ALPHA_SIZE,     8,
            GLX_DEPTH_SIZE,     depthBits,
            GLX_STENCIL_SIZE,   stencilBits,
            //GLX_SAMPLE_BUFFERS, 1,
            //GLX_SAMPLES,        1,//samples_,
            None
        };

        int screen = DefaultScreen(api_.glx.display);

        int fbCount = 0;
        GLXFBConfig* fbcList = glXChooseFBConfig(api_.glx.display, screen, fbAttribs, &fbCount);

        if (fbcList != nullptr && fbCount > 0)
        {
            int contextAttribs[] =
            {
                GLX_CONTEXT_MAJOR_VERSION_ARB, major,
                GLX_CONTEXT_MINOR_VERSION_ARB, minor,
                GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
                //GLX_CONTEXT_FLAGS_ARB      , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                None
            };

            GLXContext glc = glXCreateContextAttribsARB(api_.glx.display, fbcList[0], nullptr, True, contextAttribs);

            XFree(fbcList);

            return glc;
        }
    }

    /* Context creation failed */
    Log::Errorf("failed to create OpenGL core profile\n");

    return nullptr;
}

GLXContext LinuxGLContext::CreateGLXContextCompatibilityProfile(XVisualInfo* visual, GLXContext glcShared)
{
    /* Create compatibility profile */
    return glXCreateContext(api_.glx.display, visual, glcShared, GL_TRUE);
}

void LinuxGLContext::CreateProxyGLXContext(
    const GLPixelFormat&                    pixelFormat,
    const NativeHandle&                     nativeWindowHandle,
    const OpenGL::RenderSystemNativeHandle& nativeContextHandle)
{
    LLGL_ASSERT(nativeWindowHandle.type == NativeHandleType::X11);
    LLGL_ASSERT(nativeContextHandle.type == OpenGL::RenderSystemNativeHandleType::GLX);

    LLGL_ASSERT_PTR(nativeWindowHandle.x11.display);
    LLGL_ASSERT_PTR(nativeContextHandle.glx);

    ::Display* display = static_cast<::Display*>(nativeWindowHandle.x11.display);

    /* Get X11 display, window, and visual information */
    api_.glx.display    = display;
    api_.glx.context    = nativeContextHandle.glx;

    if (glXMakeCurrent(display, nativeWindowHandle.x11.window, api_.glx.context) != True)
        Log::Errorf("glXMakeCurrent failed on custom GLX context\n");

    /* Deduce color and depth-stencil formats */
    SetDefaultColorFormat();
    DeduceDepthStencilFormat(pixelFormat.depthBits, pixelFormat.stencilBits);
}

void LinuxGLContext::CreateProxyEGLContext(
    const GLPixelFormat&                    pixelFormat,
    const NativeHandle&                     nativeWindowHandle,
    const OpenGL::RenderSystemNativeHandle& nativeContextHandle)
{
    LLGL_ASSERT(nativeWindowHandle.type == NativeHandleType::Wayland);
    LLGL_TRAP("TODO");
}

} // /namespace LLGL



// ================================================================================
