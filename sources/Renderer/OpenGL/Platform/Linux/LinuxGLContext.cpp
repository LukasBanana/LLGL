/*
 * LinuxGLContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "LinuxGLContext.h"
#include "../../Ext/GLExtensions.h"
#include "../../Ext/GLExtensionLoader.h"
#include "../../GLCore.h"
#include "../../../CheckedCast.h"
#include "../../../../Core/Helper.h"
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
    const RenderContextDescriptor&      desc,
    const RendererConfigurationOpenGL&  config,
    Surface&                            surface,
    GLContext*                          sharedContext)
{
    LinuxGLContext* sharedContextGLX = (sharedContext != nullptr ? LLGL_CAST(LinuxGLContext*, sharedContext) : nullptr);
    return MakeUnique<LinuxGLContext>(desc, config, surface, sharedContextGLX);
}


/*
 * LinuxGLContext class
 */

LinuxGLContext::LinuxGLContext(
    const RenderContextDescriptor&      desc,
    const RendererConfigurationOpenGL&  config,
    Surface&                            surface,
    LinuxGLContext*                     sharedContext)
:
    GLContext { sharedContext }
{
    NativeHandle nativeHandle = {};
    surface.GetNativeHandle(&nativeHandle, sizeof(nativeHandle));
    CreateContext(desc, config, nativeHandle, sharedContext);
}

LinuxGLContext::~LinuxGLContext()
{
    DeleteContext();
}

bool LinuxGLContext::SetSwapInterval(int interval)
{
    /* Load GL extension "glXSwapIntervalSGI" to set v-sync interval */
    if (glXSwapIntervalSGI || LoadSwapIntervalProcs())
        return (glXSwapIntervalSGI(interval) == 0);
    else
        return false;
}

bool LinuxGLContext::SwapBuffers()
{
    glXSwapBuffers(display_, wnd_);
    return true;
}

void LinuxGLContext::Resize(const Extent2D& resolution)
{
    //TODO...
}

std::uint32_t LinuxGLContext::GetSamples() const
{
    return samples_;
}


/*
 * ======= Private: =======
 */

bool LinuxGLContext::Activate(bool activate)
{
    if (activate)
        return glXMakeCurrent(display_, wnd_, glc_);
    else
        return glXMakeCurrent(nullptr, 0, 0);
}

void LinuxGLContext::CreateContext(
    const RenderContextDescriptor&      contextDesc,
    const RendererConfigurationOpenGL&  config,
    const NativeHandle&                 nativeHandle,
    LinuxGLContext*                     sharedContext)
{
    GLXContext glcShared = (sharedContext != nullptr ? sharedContext->glc_ : nullptr);

    samples_ = contextDesc.samples;

    /* Get X11 display, window, and visual information */
    display_    = nativeHandle.display;
    wnd_        = nativeHandle.window;
    visual_     = nativeHandle.visual;

    if (!display_ || !wnd_ || !visual_)
        throw std::invalid_argument("failed to create OpenGL context on X11 client, due to missing arguments");

    /* Create intermediate GL context OpenGL context with X11 lib */
    GLXContext intermediateGlc = CreateContextCompatibilityProfile(nullptr);
    if (glXMakeCurrent(display_, wnd_, intermediateGlc) != True)
        Log::PostReport(Log::ReportType::Error, "glXMakeCurrent failed on GLX compatibility profile");

    if (config.contextProfile == OpenGLContextProfile::CoreProfile)
    {
        /* Create core profile */
        glc_ = CreateContextCoreProfile(glcShared, config.majorVersion, config.minorVersion);
    }

    if (glc_)
    {
        /* Make new OpenGL context current */
        if (glXMakeCurrent(display_, wnd_, glc_) != True)
            Log::PostReport(Log::ReportType::Error, "glXMakeCurrent failed on GLX core profile");

        /* Valid core profile created, so we can delete the intermediate GLX context */
        glXDestroyContext(display_, intermediateGlc);
    }
    else
    {
        /* No core profile created, so we use the intermediate GLX context */
        glc_ = intermediateGlc;
    }
}

void LinuxGLContext::DeleteContext()
{
    glXDestroyContext(display_, glc_);
}

GLXContext LinuxGLContext::CreateContextCoreProfile(GLXContext glcShared, int major, int minor)
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
            GLX_DEPTH_SIZE,     24,
            GLX_STENCIL_SIZE,   8,
            //GLX_SAMPLE_BUFFERS, 1,
            //GLX_SAMPLES,        1,//static_cast<int>(desc_.samples),
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

GLXContext LinuxGLContext::CreateContextCompatibilityProfile(GLXContext glcShared)
{
    /* Create compatibility profile */
    return glXCreateContext(display_, visual_, glcShared, GL_TRUE);
}


} // /namespace LLGL



// ================================================================================
