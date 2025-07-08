/*
 * LinuxGLContextX11.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "LinuxGLContextX11.h"
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
 * LinuxGLContextX11 class
 */

LinuxGLContextX11::LinuxGLContextX11(
    const GLPixelFormat&                    pixelFormat,
    const RendererConfigurationOpenGL&      profile,
    Surface&                                surface,
    LinuxGLContextX11*                      sharedContext,
    const OpenGL::RenderSystemNativeHandle* customNativeHandle)
:
    samples_ { pixelFormat.samples }
{
    /* Notify the shared X11 display that it'll be used by libGL.so to ensure a clean teardown */
    LinuxSharedX11Display::RetainLibGL();

    /* Create GLX or proxy context if a custom one is specified */
    NativeHandle nativeWindowHandle = {};
    surface.GetNativeHandle(&nativeWindowHandle, sizeof(nativeWindowHandle));
    if (customNativeHandle != nullptr)
    {
        isProxyGLC_ = true;
        CreateProxyContext(pixelFormat, nativeWindowHandle, *customNativeHandle);
    }
    else
        CreateGLXContext(pixelFormat, profile, nativeWindowHandle, sharedContext);
}

LinuxGLContextX11::~LinuxGLContextX11()
{
    if (!isProxyGLC_)
        DeleteGLXContext();
}

int LinuxGLContextX11::GetSamples() const
{
    return samples_;
}

bool LinuxGLContextX11::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(OpenGL::RenderSystemNativeHandle))
    {
        auto* nativeHandleGL = static_cast<OpenGL::RenderSystemNativeHandle*>(nativeHandle);

        nativeHandleGL->type    = OpenGL::RenderSystemNativeType::GLX;
        nativeHandleGL->glx     = glc_;
        return true;
    }
    return false;
}

OpenGL::RenderSystemNativeType LinuxGLContextX11::GetNativeType() const
{
    return OpenGL::RenderSystemNativeType::GLX;
}

::XVisualInfo* LinuxGLContextX11::ChooseVisual(::Display* display, int screen, const GLPixelFormat& pixelFormat, int& outSamples)
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

bool LinuxGLContextX11::SetSwapInterval(int interval)
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

void LinuxGLContextX11::CreateGLXContext(
    const GLPixelFormat&                pixelFormat,
    const RendererConfigurationOpenGL&  profile,
    const NativeHandle&                 nativeHandle,
    LinuxGLContextX11*                     sharedContext)
{
    LLGL_ASSERT_PTR(nativeHandle.x11.display);
    LLGL_ASSERT_PTR(nativeHandle.x11.window);

    GLXContext glcShared = (sharedContext != nullptr ? sharedContext->glc_ : nullptr);

    /* Get X11 display, window, and visual information */
    display_ = nativeHandle.x11.display;

    /* Ensure GLX is a supported X11 extension */
    int errorBase = 0, eventBase = 0;
    if (glXQueryExtension(display_, &errorBase, &eventBase) == False)
        LLGL_TRAP("GLX extension is not supported by X11 implementation");

    /* Get X11 visual information or choose it now */
    ::XVisualInfo* visual = nativeHandle.x11.visual;
    if (visual == nullptr)
    {
        visual = LinuxGLContextX11::ChooseVisual(display_, nativeHandle.x11.screen, pixelFormat, samples_);
        LLGL_ASSERT(visual != nullptr, "failed to choose X11VisualInfo");
    }

    /* Create intermediate GL context OpenGL context with X11 lib */
    GLXContext intermediateGlc = CreateGLXContextCompatibilityProfile(visual, nullptr);

    if (glXMakeCurrent(display_, nativeHandle.x11.window, intermediateGlc) != True)
        Log::Errorf("glXMakeCurrent failed on GLX compatibility profile\n");

    if (profile.contextProfile == OpenGLContextProfile::CoreProfile)
    {
        /* Create core profile */
        glc_ = CreateGLXContextCoreProfile(glcShared, profile.majorVersion, profile.minorVersion, pixelFormat.depthBits, pixelFormat.stencilBits);
    }

    if (glc_)
    {
        /* Make new OpenGL context current */
        if (glXMakeCurrent(display_, nativeHandle.x11.window, glc_) != True)
            Log::Errorf("glXMakeCurrent failed on GLX core profile\n");

        /* Valid core profile created, so we can delete the intermediate GLX context */
        glXDestroyContext(display_, intermediateGlc);

        /* Deduce color and depth-stencil formats */
        SetDefaultColorFormat();
        DeduceDepthStencilFormat(pixelFormat.depthBits, pixelFormat.stencilBits);
    }
    else
    {
        /* No core profile created, so we use the intermediate GLX context */
        glc_ = intermediateGlc;

        /* Set fixed color and depth-stencil formats as default values */
        SetDefaultColorFormat();
        SetDefaultDepthStencilFormat();
    }
}

void LinuxGLContextX11::DeleteGLXContext()
{
    glXMakeCurrent(display_, None, nullptr);
    glXDestroyContext(display_, glc_);
}

GLXContext LinuxGLContextX11::CreateGLXContextCoreProfile(GLXContext glcShared, int major, int minor, int depthBits, int stencilBits)
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

        int screen = DefaultScreen(display_);

        int fbCount = 0;
        GLXFBConfig* fbcList = glXChooseFBConfig(display_, screen, fbAttribs, &fbCount);

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

            GLXContext glc = glXCreateContextAttribsARB(display_, fbcList[0], nullptr, True, contextAttribs);

            XFree(fbcList);

            return glc;
        }
    }

    /* Context creation failed */
    Log::Errorf("failed to create OpenGL core profile\n");

    return nullptr;
}

GLXContext LinuxGLContextX11::CreateGLXContextCompatibilityProfile(XVisualInfo* visual, GLXContext glcShared)
{
    /* Create compatibility profile */
    return glXCreateContext(display_, visual, glcShared, GL_TRUE);
}

void LinuxGLContextX11::CreateProxyContext(
    const GLPixelFormat&                    pixelFormat,
    const NativeHandle&                     nativeWindowHandle,
    const OpenGL::RenderSystemNativeHandle& nativeContextHandle)
{
    LLGL_ASSERT_PTR(nativeWindowHandle.x11.display);
    LLGL_ASSERT_PTR(nativeContextHandle.glx);

    /* Get X11 display, window, and visual information */
    display_    = nativeWindowHandle.x11.display;
    glc_        = nativeContextHandle.glx;

    if (glXMakeCurrent(display_, nativeWindowHandle.x11.window, glc_) != True)
        Log::Errorf("glXMakeCurrent failed on custom GLX context\n");

    /* Deduce color and depth-stencil formats */
    SetDefaultColorFormat();
    DeduceDepthStencilFormat(pixelFormat.depthBits, pixelFormat.stencilBits);
}


} // /namespace LLGL



// ================================================================================