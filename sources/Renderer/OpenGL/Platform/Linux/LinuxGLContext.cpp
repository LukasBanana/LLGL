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
    if (customNativeHandle != nullptr)
    {
        isProxyGLC_ = true;
        CreateProxyContext(pixelFormat, nativeWindowHandle, *customNativeHandle);
    }
    else
        CreateGLXContext(pixelFormat, profile, nativeWindowHandle, sharedContext);
}

LinuxGLContext::~LinuxGLContext()
{
    if (!isProxyGLC_)
        DeleteGLXContext();
}

int LinuxGLContext::GetSamples() const
{
    return samples_;
}

bool LinuxGLContext::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(OpenGL::RenderSystemNativeHandle))
    {
        auto* nativeHandleGL = reinterpret_cast<OpenGL::RenderSystemNativeHandle*>(nativeHandle);
        nativeHandleGL->context = glc_;
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
    /* Load GL extension "glXSwapIntervalSGI" to set v-sync interval */
    if (glXSwapIntervalSGI || LoadSwapIntervalProcs())
        return (glXSwapIntervalSGI(interval) == 0);
    else
        return false;
}

void LinuxGLContext::CreateGLXContext(
    const GLPixelFormat&                pixelFormat,
    const RendererConfigurationOpenGL&  profile,
    const NativeHandle&                 nativeHandle,
    LinuxGLContext*                     sharedContext)
{
    LLGL_ASSERT_PTR(nativeHandle.display);
    LLGL_ASSERT_PTR(nativeHandle.window);

    GLXContext glcShared = (sharedContext != nullptr ? sharedContext->glc_ : nullptr);

    /* Get X11 display, window, and visual information */
    display_ = nativeHandle.display;

    /* Ensure GLX is a supported X11 extension */
    int errorBase = 0, eventBase = 0;
    if (glXQueryExtension(display_, &errorBase, &eventBase) == False)
        LLGL_TRAP("GLX extension is not supported by X11 implementation");

    /* Get X11 visual information or choose it now */
    ::XVisualInfo* visual = nativeHandle.visual;
    if (visual == nullptr)
    {
        visual = LinuxGLContext::ChooseVisual(display_, nativeHandle.screen, pixelFormat, samples_);
        LLGL_ASSERT(visual != nullptr, "failed to choose X11VisualInfo");
    }

    /* Create intermediate GL context OpenGL context with X11 lib */
    GLXContext intermediateGlc = CreateGLXContextCompatibilityProfile(visual, nullptr);

    if (glXMakeCurrent(display_, nativeHandle.window, intermediateGlc) != True)
        Log::Errorf("glXMakeCurrent failed on GLX compatibility profile\n");

    if (profile.contextProfile == OpenGLContextProfile::CoreProfile)
    {
        /* Create core profile */
        glc_ = CreateGLXContextCoreProfile(glcShared, profile.majorVersion, profile.minorVersion, pixelFormat.depthBits, pixelFormat.stencilBits);
    }

    if (glc_)
    {
        /* Make new OpenGL context current */
        if (glXMakeCurrent(display_, nativeHandle.window, glc_) != True)
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

void LinuxGLContext::DeleteGLXContext()
{
    glXMakeCurrent(display_, None, nullptr);
    glXDestroyContext(display_, glc_);
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
        Log::Errorf("cannot create OpenGL core profile with GL version %d.%d", major, minor);
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
    Log::Errorf("failed to create OpenGL core profile");

    return nullptr;
}

GLXContext LinuxGLContext::CreateGLXContextCompatibilityProfile(XVisualInfo* visual, GLXContext glcShared)
{
    /* Create compatibility profile */
    return glXCreateContext(display_, visual, glcShared, GL_TRUE);
}

void LinuxGLContext::CreateProxyContext(
    const GLPixelFormat&                    pixelFormat,
    const NativeHandle&                     nativeWindowHandle,
    const OpenGL::RenderSystemNativeHandle& nativeContextHandle)
{
    LLGL_ASSERT_PTR(nativeWindowHandle.display);
    LLGL_ASSERT_PTR(nativeContextHandle.context);

    /* Get X11 display, window, and visual information */
    display_    = nativeWindowHandle.display;
    glc_        = nativeContextHandle.context;

    if (glXMakeCurrent(display_, nativeWindowHandle.window, glc_) != True)
        Log::Errorf("glXMakeCurrent failed on custom GLX context\n");

    /* Deduce color and depth-stencil formats */
    SetDefaultColorFormat();
    DeduceDepthStencilFormat(pixelFormat.depthBits, pixelFormat.stencilBits);
}


} // /namespace LLGL



// ================================================================================
