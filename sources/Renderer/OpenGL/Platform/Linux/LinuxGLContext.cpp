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
#include "../../../../Core/CoreUtils.h"
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

std::unique_ptr<GLContext> GLContext::Create(
    const GLPixelFormat&                pixelFormat,
    const RendererConfigurationOpenGL&  profile,
    Surface&                            surface,
    GLContext*                          sharedContext)
{
    LinuxGLContext* sharedContextGLX = (sharedContext != nullptr ? LLGL_CAST(LinuxGLContext*, sharedContext) : nullptr);
    return MakeUnique<LinuxGLContext>(pixelFormat, profile, surface, sharedContextGLX);
}


/*
 * LinuxGLContext class
 */

LinuxGLContext::LinuxGLContext(
    const GLPixelFormat&                pixelFormat,
    const RendererConfigurationOpenGL&  profile,
    Surface&                            surface,
    LinuxGLContext*                     sharedContext)
:
    samples_ { pixelFormat.samples }
{
    NativeHandle nativeHandle = {};
    surface.GetNativeHandle(&nativeHandle, sizeof(nativeHandle));
    CreateContext(pixelFormat, profile, nativeHandle, sharedContext);
}

LinuxGLContext::~LinuxGLContext()
{
    DeleteContext();
}

void LinuxGLContext::Resize(const Extent2D& resolution)
{
    //TODO...
}

int LinuxGLContext::GetSamples() const
{
    return samples_;
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

void LinuxGLContext::CreateContext(
    const GLPixelFormat&                pixelFormat,
    const RendererConfigurationOpenGL&  profile,
    const NativeHandle&                 nativeHandle,
    LinuxGLContext*                     sharedContext)
{
    if (!nativeHandle.display || !nativeHandle.window || !nativeHandle.visual)
        throw std::invalid_argument("failed to create OpenGL context on X11 client, due to missing arguments");

    GLXContext glcShared = (sharedContext != nullptr ? sharedContext->glc_ : nullptr);

    /* Get X11 display, window, and visual information */
    display_ = nativeHandle.display;

    /* Create intermediate GL context OpenGL context with X11 lib */
    GLXContext intermediateGlc = CreateContextCompatibilityProfile(nativeHandle.visual, nullptr);

    if (glXMakeCurrent(display_, nativeHandle.window, intermediateGlc) != True)
        Log::PostReport(Log::ReportType::Error, "glXMakeCurrent failed on GLX compatibility profile");

    if (profile.contextProfile == OpenGLContextProfile::CoreProfile)
    {
        /* Create core profile */
        glc_ = CreateContextCoreProfile(glcShared, profile.majorVersion, profile.minorVersion, pixelFormat.depthBits, pixelFormat.stencilBits);
    }

    if (glc_)
    {
        /* Make new OpenGL context current */
        if (glXMakeCurrent(display_, nativeHandle.window, glc_) != True)
            Log::PostReport(Log::ReportType::Error, "glXMakeCurrent failed on GLX core profile");

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

void LinuxGLContext::DeleteContext()
{
    glXDestroyContext(display_, glc_);
}

GLXContext LinuxGLContext::CreateContextCoreProfile(GLXContext glcShared, int major, int minor, int depthBits, int stencilBits)
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
        Log::PostReport(
            Log::ReportType::Error,
            "cannot create OpenGL core profile with GL version " +
            std::to_string(major) + '.' + std::to_string(minor)
        );
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

            auto glc = glXCreateContextAttribsARB(display_, fbcList[0], nullptr, True, contextAttribs);

            XFree(fbcList);

            return glc;
        }
    }

    /* Context creation failed */
    Log::PostReport(Log::ReportType::Error, "failed to create OpenGL core profile");

    return nullptr;
}

GLXContext LinuxGLContext::CreateContextCompatibilityProfile(XVisualInfo* visual, GLXContext glcShared)
{
    /* Create compatibility profile */
    return glXCreateContext(display_, visual, glcShared, GL_TRUE);
}


} // /namespace LLGL



// ================================================================================
