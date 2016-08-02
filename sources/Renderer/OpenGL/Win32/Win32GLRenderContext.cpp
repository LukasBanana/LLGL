/*
 * Win32GLRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../GLRenderContext.h"
#include "../GLExtensions.h"
#include "../GLExtensionLoader.h"
#include <LLGL/Log.h>
#include <algorithm>


namespace LLGL
{


void GLRenderContext::Present()
{
    SwapBuffers(context_.hDC);
}


/*
 * ======= Private: =======
 */

static void ErrAntiAliasingNotSupported()
{
    Log::StdErr() << "multi-sample anti-aliasing is not supported" << std::endl;
}

/*
TODO:
- When anti-aliasing and extended-profile-selection is enabled,
  maximal 2 contexts should be created (and not 3).
*/
void GLRenderContext::CreateContext(GLRenderContext* sharedRenderContext)
{
    /* If a shared context has passed, use its pre-selected pixel format */
    if (desc_.antiAliasing.enabled && sharedRenderContext)
        CopyPixelFormat(*sharedRenderContext);

    /* First setup device context and choose pixel format */
    SetupDeviceContextAndPixelFormat();

    /* Create standard render context first */
    auto stdRenderContext = CreateGLContext(false, sharedRenderContext);

    if (!stdRenderContext)
        throw std::runtime_error("failed to create standard OpenGL render context");

    /* Check for multi-sample anti-aliasing */
    if (desc_.antiAliasing.enabled && !hasSharedContext_)
    {
        /* Setup anti-aliasing after creating a standard render context. */
        if (SetupAntiAliasing())
        {
            /* Delete old standard render context */
            DeleteGLContext(stdRenderContext);

            /*
            For anti-aliasing we must recreate the window,
            because a pixel format can be choosen only once for a Win32 window,
            then update device context and pixel format
            */
            RecreateWindow();

            /* Create a new render context -> now with anti-aliasing pixel format */
            stdRenderContext = CreateGLContext(false, sharedRenderContext);

            if (!stdRenderContext)
                Log::StdErr() << "failed to create multi-sample anti-aliasing" << std::endl;
        }
        else
        {
            /* Print warning and disable anti-aliasing */
            ErrAntiAliasingNotSupported();

            desc_.antiAliasing.enabled = false;
            desc_.antiAliasing.samples = 0;
        }
    }

    context_.hGLRC = stdRenderContext;

    /* Check for extended render context */
    if (desc_.profileOpenGL.extProfile && !hasSharedContext_)
    {
        /*
        Load profile selection extension (wglCreateContextAttribsARB) via current context,
        then create new context with extended settings.
        */
        if (wglCreateContextAttribsARB || LoadCreateContextProcs())
        {
            auto extRenderContext = CreateGLContext(true, sharedRenderContext);
            
            if (extRenderContext)
            {
                /* Use the extended profile and delete the old standard render context */
                context_.hGLRC = extRenderContext;
                DeleteGLContext(stdRenderContext);
            }
            else
            {
                /* Print warning and disbale profile selection */
                Log::StdErr() << "failed to create extended OpenGL profile" << std::endl;
                desc_.profileOpenGL.extProfile = false;
            }
        }
        else
        {
            /* Print warning and disable profile settings */
            Log::StdErr() << "failed to select OpenGL profile" << std::endl;
            desc_.profileOpenGL.extProfile = false;
        }
    }

    /* Check if context creation was successful */
    if (!context_.hGLRC)
        throw std::runtime_error("failed to create OpenGL render context");

    if (wglMakeCurrent(context_.hDC, context_.hGLRC) != TRUE)
        throw std::runtime_error("failed to activate OpenGL render context");

    /*
    Share resources with previous render context (only for compatibility profile).
    -> Only do this, if this context has its own GL hardware context (hasSharedContext_ == false),
       but a shared render context was passed (sharedRenderContext != null).
    */
    if (sharedRenderContext && !hasSharedContext_ && !desc_.profileOpenGL.extProfile)
    {
        if (!wglShareLists(sharedRenderContext->context_.hGLRC, context_.hGLRC))
            throw std::runtime_error("failed to share resources from OpenGL render context");
    }

    /* Query GL version of final render context */
    //QueryGLVersion();

    /* Setup v-sync interval */
    SetupVSyncInterval();
}

void GLRenderContext::DeleteContext()
{
    if (!hasSharedContext_)
    {
        /* Deactivate context before deletion */
        //Deactivate();
        DeleteGLContext(context_.hGLRC);
    }
}

void GLRenderContext::DeleteGLContext(HGLRC& renderContext)
{
    /* Delete GL render context */
    if (!wglDeleteContext(renderContext))
        Log::StdErr() << "failed to delete OpenGL render context" << std::endl;
    else
        renderContext = 0;
}

HGLRC GLRenderContext::CreateGLContext(bool useExtProfile, GLRenderContext* sharedRenderContext)
{
    /* Create hardware render context */
    HGLRC renderContext = 0;

    if (!sharedRenderContext || !sharedRenderContext->context_.hGLRC /* || createOwnHardwareContext == true*/)
    {
        /* Create own hardware context */
        hasSharedContext_ = false;

        if (useExtProfile)
        {
            renderContext = CreateExtContextProfile(
                (sharedRenderContext != nullptr ? sharedRenderContext->context_.hGLRC : nullptr)
            );
        }
        else
            renderContext = CreateStdContextProfile();
    }
    else
    {
        /* Use shared render context */
        hasSharedContext_ = true;
        renderContext = sharedRenderContext->context_.hGLRC;
    }
    
    if (!renderContext)
        return 0;
        
    /* Activate new render context */
    if (wglMakeCurrent(context_.hDC, renderContext) != TRUE)
    {
        /* Print error and delete unusable render context */
        Log::StdErr() << "failed to active OpenGL render context" << std::endl;
        DeleteGLContext(renderContext);
        return 0;
    }

    /* Query GL version of current render context */
    //QueryGLVersion();

    return renderContext;
}

HGLRC GLRenderContext::CreateStdContextProfile()
{
    /* Create OpenGL "Compatibility Profile" render context */
    return wglCreateContext(context_.hDC);
}

static void ConvertGLVersion(const OpenGLVersion version, GLint& major, GLint& minor)
{
    if (version == OpenGLVersion::OpenGL_Latest)
    {
        major = 4;
        minor = 5;
    }
    else
    {
        auto ver = static_cast<int>(version);
        major = ver / 100;
        minor = (ver % 100) / 10;
    }
}

HGLRC GLRenderContext::CreateExtContextProfile(HGLRC sharedGLRC)
{
    bool useCoreProfile = desc_.profileOpenGL.coreProfile;
    
    /* Initialize GL version number */
    GLint major = 0, minor = 0;
    ConvertGLVersion(desc_.profileOpenGL.version, major, minor);

    /* Setup extended attributes to select the OpenGL profile */
    const int attribList[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB,  major,
        WGL_CONTEXT_MINOR_VERSION_ARB,  minor,
        #ifdef LLGL_DEBUG
        WGL_CONTEXT_FLAGS_ARB,          WGL_CONTEXT_DEBUG_BIT_ARB /*| WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB*/,
        #endif
        WGL_CONTEXT_PROFILE_MASK_ARB,   (useCoreProfile ? WGL_CONTEXT_CORE_PROFILE_BIT_ARB : WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB),
        0, 0
    };

    /* Create OpenGL "Core Profile" or "Compatibility Profile" render context */
    HGLRC renderContext = wglCreateContextAttribsARB(context_.hDC, sharedGLRC, attribList);

    /* Check for errors */
    DWORD error = GetLastError();

    if (error == ERROR_INVALID_VERSION_ARB)
        Log::StdErr() << "invalid version for OpenGL profile" << std::endl;
    else if (error == ERROR_INVALID_PROFILE_ARB)
        Log::StdErr() << "invalid OpenGL profile" << std::endl;
    else
        return renderContext;

    return 0;
}

void GLRenderContext::SetupDeviceContextAndPixelFormat()
{
    /* Get device context from window */
    HWND wnd = *reinterpret_cast<const HWND*>(GetWindow().GetNativeHandle());
    context_.hDC = GetDC(wnd);

    /* Select suitable pixel format */
    SelectPixelFormat();
}

void GLRenderContext::SelectPixelFormat()
{
    /* Setup pixel format attributes */
    const auto colorDepth = static_cast<BYTE>(desc_.videoMode.colorDepth);

    PIXELFORMATDESCRIPTOR formatDesc
    {
        sizeof(PIXELFORMATDESCRIPTOR),  // Structure size
        1,                              // Version number
        ( PFD_DRAW_TO_WINDOW |          // Format must support draw-to-window
          PFD_SUPPORT_OPENGL |          // Format must support OpenGL
          PFD_DOUBLEBUFFER   |          // Must support double buffering
          PFD_SWAP_EXCHANGE ),          // Hint to the driver to exchange the back- with the front buffer
        PFD_TYPE_RGBA,                  // Request an RGBA format
        colorDepth,                     // Select color bit depth (cColorBits)
        0, 0, 0, 0, 0, 0,               // Color bits ignored
        8,                              // Request an alpha buffer of 8 bits (cAlphaBits)
        0,                              // Shift bit ignored
        0,                              // No accumulation buffer
        0, 0, 0, 0,                     // Accumulation bits ignored
        24,                             // Z-Buffer bits (cDepthBits)
        1,                              // Stencil buffer bits (cStencilBits)
        0,                              // No auxiliary buffer
        0,                              // Main drawing layer (No longer used)
        0,                              // Reserved
        0, 0, 0                         // Layer masks ignored
    };
    
    /* Try to find suitable pixel format */
    const bool wantAntiAliasFormat = (desc_.antiAliasing.enabled && !context_.pixelFormatsMS.empty());

    std::size_t msPixelFormatIndex = 0;
    bool wasStandardFormatUsed = false;

    while (true)
    {
        if (wantAntiAliasFormat && msPixelFormatIndex < GLPlatformContext::maxNumPixelFormatsMS)
        {
            /* Choose anti-aliasing pixel format */
            context_.pixelFormat = context_.pixelFormatsMS[msPixelFormatIndex++];
        }
        
        if (!context_.pixelFormat)
        {
            /* Choose standard pixel format */
            context_.pixelFormat = ChoosePixelFormat(context_.hDC, &formatDesc);
            
            if (wantAntiAliasFormat)
                ErrAntiAliasingNotSupported();
            
            wasStandardFormatUsed = true;
        }
        
        /* Check for errors */
        if (!context_.pixelFormat)
            throw std::runtime_error("failed to select pixel format");
        
        /* Set pixel format */
        auto wasFormatSelected = SetPixelFormat(context_.hDC, context_.pixelFormat, &formatDesc);
        
        if (!wasFormatSelected)
        {
            if (wasStandardFormatUsed)
                throw std::runtime_error("failed to set pixel format");
        }
        else
        {
            /* Format was selected -> quit with success */
            break;
        }
    }
}

bool GLRenderContext::SetupAntiAliasing()
{
    /*
    Load GL extension "wglChoosePixelFormatARB" to choose anti-aliasing pixel formats
    A valid (standard) GL context must be created at this time, before an extension can be loaded!
    */
    if (!wglChoosePixelFormatARB && !LoadPixelFormatProcs())
        return false;

    /* Setup pixel format for anti-aliasing */
    const auto queriedMultiSamples = desc_.antiAliasing.samples;

    while (desc_.antiAliasing.samples > 0)
    {
        float attribsFlt[] = { 0.0f, 0.0f };

        int attribsInt[] =
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
            WGL_COLOR_BITS_ARB,     desc_.videoMode.colorDepth,
            WGL_ALPHA_BITS_ARB,     8,
            WGL_DEPTH_BITS_ARB,     24,
            WGL_STENCIL_BITS_ARB,   1,
            WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
            WGL_SAMPLE_BUFFERS_ARB, (desc_.antiAliasing.enabled ? GL_TRUE : GL_FALSE),
            WGL_SAMPLES_ARB,        static_cast<int>(desc_.antiAliasing.samples),
            0, 0
        };

        /* Choose new pixel format with anti-aliasing */
        UINT numFormats = 0;

        context_.pixelFormatsMS.resize(GLPlatformContext::maxNumPixelFormatsMS);

        int result = wglChoosePixelFormatARB(
            context_.hDC,
            attribsInt,
            attribsFlt,
            GLPlatformContext::maxNumPixelFormatsMS,
            context_.pixelFormatsMS.data(),
            &numFormats
        );

        context_.pixelFormatsMS.resize(numFormats);

        if (!result || numFormats < 1)
        {
            if (desc_.antiAliasing.samples <= 0)
            {
                /* Lowest count of multi-samples reached -> return with error */
                return false;
            }

            /* Choose next lower count of multi-samples */
            --desc_.antiAliasing.samples;
        }
        else
        {
            /* Found suitable pixel formats */
            break;
        }
    }

    /* Check if multi-sample count was reduced */
    if (desc_.antiAliasing.samples < queriedMultiSamples)
    {
        Log::StdOut()
            << "reduced multi-samples for anti-aliasing from "
            << std::to_string(queriedMultiSamples) << " to "
            << std::to_string(desc_.antiAliasing.samples) << std::endl;
    }

    /* Enable anti-aliasing */
    if (desc_.antiAliasing.enabled)
        glEnable(GL_MULTISAMPLE);
    else
        glDisable(GL_MULTISAMPLE);

    return true;
}

void GLRenderContext::CopyPixelFormat(GLRenderContext& sourceContext)
{
    context_.pixelFormat    = sourceContext.context_.pixelFormat;
    context_.pixelFormatsMS = sourceContext.context_.pixelFormatsMS;
}

bool GLRenderContext::SetupVSyncInterval()
{
    /* Load GL extension "wglSwapIntervalEXT" to set v-sync interval */
    if (wglSwapIntervalEXT || LoadSwapIntervalProcs())
    {
        /* Setup v-sync interval */
        int interval = (desc_.vsync.enabled ? static_cast<int>(desc_.vsync.interval) : 0);
        wglSwapIntervalEXT(interval);
        return true;
    }
    return false;
}

void GLRenderContext::RecreateWindow()
{
    /* Recreate window with current descriptor, then update device context and pixel format */
    window_->Recreate(window_->QueryDesc());
    SetupDeviceContextAndPixelFormat();
}


} // /namespace LLGL



// ================================================================================
