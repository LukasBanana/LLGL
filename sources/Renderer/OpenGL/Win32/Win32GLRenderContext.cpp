/*
 * Win32GLRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../GLRenderContext.h"


namespace LLGL
{


void GLRenderContext::Present()
{
    SwapBuffers(context_.hDC);
}


/*
 * ======= Private: =======
 */

#if 1

void GLRenderContext::CreateContext(GLRenderContext* sharedRenderContext)
{
}

void GLRenderContext::DeleteContext()
{
}

#else

/*
TODO:
- When anti-aliasing and extended-profile-selection is enabled,
  maximal 2 contexts should be created (and not 3).
*/
void GLRenderContext::CreateContext(GLRenderContext* sharedRenderContext)
{
    /* If a shared context has passed, use its pre-selected pixel format */
    if (contextDesc_.antiAliasingDesc.isEnabled && sharedRenderContext)
        CopyPixelFormat(sharedRenderContext);

    /* First setup device context and choose pixel format */
    SetupDeviceContextAndPixelFormat();

    /* Create standard render context first */
    auto stdRenderContext = CreateGLContext(false, sharedRenderContext);

    if (!stdRenderContext)
        throw RenderContextException(ERR_GL_CONTEXT_FAILED("Creating standard"));

    /* Check for multi-sample anti-aliasing */
    if (contextDesc_.antiAliasingDesc.isEnabled && !hasSharedContext_)
    {
        /* Setup anti-aliasing after creating a standard render context. */
        if (SetupAntiAliasing())
        {
            /* Delete old standard render context */
            DeleteGLContext(stdRenderContext);

            ReCreateFrameAndUpdatePixelFormat();

            /* Create a new render context -> now with anti-aliasing pixel format */
            stdRenderContext = CreateGLContext(false, sharedRenderContext);

            if (!stdRenderContext)
                IO::Log::Warning(ERR_GL_CONTEXT_FAILED("Creating multi-sample anti-aliasing"));
        }
        else
        {
            /* Print warning and disable anti-aliasing */
            IO::Log::Warning(errAntiAliasingNotSupported);

            contextDesc_.antiAliasingDesc.isEnabled     = false;
            contextDesc_.antiAliasingDesc.multiSamples  = 0;
        }
    }

    renderContext_ = stdRenderContext;

    /* Check for extended render context */
    if (contextDesc_.rendererProfileDesc.useExtProfile && !hasSharedContext_)
    {
        /*
        Load profile selection extension (wglCreateContextAttribsARB) via current context,
        then create new context with extended settings.
        */
        if (wglCreateContextAttribsARB || GLExtensionLoader::LoadCreateContextProcs())
        {
            auto extRenderContext = CreateGLContext(true, sharedRenderContext);
            
            if (extRenderContext)
            {
                /* Use the extended profile and delete the old standard render context */
                renderContext_ = extRenderContext;
                DeleteGLContext(stdRenderContext);
            }
            else
            {
                /* Print warning and disbale profile selection */
                IO::Log::Warning("Extended OpenGL profile creation failed");
                contextDesc_.rendererProfileDesc.useExtProfile = false;
            }
        }
        else
        {
            /* Print warning and disable profile settings */
            IO::Log::Warning("OpenGL profile selection is not supported");
            contextDesc_.rendererProfileDesc.useExtProfile = false;
        }
    }

    /* Check if context creation was successful */
    if (!renderContext_)
        throw RenderContextException(ERR_GL_CONTEXT_FAILED("Creating"));

    if (!Activate())
        throw RenderContextException(ERR_GL_CONTEXT_FAILED("Activating"));

    /*
    Share resources with previous render context (only for compatibility profile).
    -> Only do this, if this context has its own GL hardware context (hasSharedContext_ == false),
       but a shared render context was passed (sharedRenderContext != null).
    */
    if (sharedRenderContext && !hasSharedContext_ && !contextDesc_.rendererProfileDesc.useExtProfile)
    {
        if (!wglShareLists(sharedRenderContext->renderContext_, renderContext_))
            throw RenderContextException(ERR_GL_CONTEXT_FAILED("Resource sharing for "));
    }

    /* Query GL version of final render context */
    QueryGLVersion();

    /* Setup v-sync interval */
    SetupVSyncInterval();
}

void GLRenderContext::DeleteContext()
{
    if (!hasSharedContext_)
    {
        /* Deactivate context before deletion */
        Deactivate();
        DeleteGLContext(renderContext_);
    }
}

void GLRenderContext::DeleteGLContext(HGLRC& renderContext)
{
    /* Delete GL render context */
    if (!wglDeleteContext(renderContext))
        IO::Log::Error(ERR_GL_CONTEXT_FAILED("Deleting"));
    else
        renderContext = 0;
}

HGLRC GLRenderContext::CreateGLContext(bool useExtProfile, GLRenderContext* sharedRenderContext)
{
    /* Create hardware render context */
    HGLRC renderContext = 0;

    if (!sharedRenderContext || !sharedRenderContext->renderContext_ /* || createOwnHardwareContext == true*/)
    {
        /* Create own hardware context */
        hasSharedContext_ = false;

        if (useExtProfile)
        {
            renderContext = CreateExtContextProfile(
                sharedRenderContext != nullptr ? sharedRenderContext->renderContext_ : nullptr
            );
        }
        else
            renderContext = CreateStdContextProfile();
    }
    else
    {
        /* Use shared render context */
        hasSharedContext_ = true;
        renderContext = sharedRenderContext->renderContext_;
    }
    
    if (!renderContext)
        return 0;
        
    /* Activate new render context */
    if (wglMakeCurrent(deviceContext_, renderContext) != TRUE)
    {
        /* Print error and delete unusable render context */
        IO::Log::Error(ERR_GL_CONTEXT_FAILED("Activating"));
        DeleteGLContext(renderContext);
        return 0;
    }

    /* Query GL version of current render context */
    QueryGLVersion();

    return renderContext;
}

HGLRC GLRenderContext::CreateStdContextProfile()
{
    /* Create OpenGL "Compatibility Profile" render context */
    return wglCreateContext(deviceContext_);
}

HGLRC GLRenderContext::CreateExtContextProfile(HGLRC sharedGLRC)
{
    bool useCoreProfile = contextDesc_.rendererProfileDesc.useGLCoreProfile;
    
    /* Initialize GL version number */
    const auto& versionDesc = contextDesc_.rendererProfileDesc.versionGL;

    GLVersion versionGL = {
        static_cast<int>(versionDesc.major),
        static_cast<int>(versionDesc.minor)
    };

    /* <Bitwise OR> for fast zero comparision */
    if ((versionGL.major | versionGL.minor) == 0)
        versionGL = versionGL_;

    /* Setup extended attributes to select the OpenGL profile */
    const int attribList[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB,  versionGL.major,
        WGL_CONTEXT_MINOR_VERSION_ARB,  versionGL.minor,
        #ifdef FORK_DEBUG
        WGL_CONTEXT_FLAGS_ARB,          WGL_CONTEXT_DEBUG_BIT_ARB /*| WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB*/,
        #endif
        WGL_CONTEXT_PROFILE_MASK_ARB,   (useCoreProfile ? WGL_CONTEXT_CORE_PROFILE_BIT_ARB : WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB),
        0, 0
    };

    /* Create OpenGL "Core Profile" or "Compatibility Profile" render context */
    HGLRC renderContext = wglCreateContextAttribsARB(
        deviceContext_, sharedGLRC, attribList
    );

    /* Check for errors */
    DWORD error = GetLastError();

    if (error == ERROR_INVALID_VERSION_ARB)
        IO::Log::Error("Invalid version for OpenGL profile");
    else if (error == ERROR_INVALID_PROFILE_ARB)
        IO::Log::Error("Invalid OpenGL profile");
    else
        return renderContext;

    return 0;
}

void GLRenderContext::SetupDeviceContextAndPixelFormat()
{
    /* Get device context from frame object */
    deviceContext_ = *reinterpret_cast<const HDC*>(frame_->GetNativeDeviceContext());

    /* Select suitable pixel format */
    SelectPixelFormat();
}

void GLRenderContext::SelectPixelFormat()
{
    /* Setup pixel format attributes */
    PIXELFORMATDESCRIPTOR formatDesc
    {
        sizeof(PIXELFORMATDESCRIPTOR),                      // Structure size
        1,                                                  // Version number
        ( PFD_DRAW_TO_WINDOW |                              // Format must support draw-to-window
          PFD_SUPPORT_OPENGL |                              // Format must support OpenGL
          PFD_DOUBLEBUFFER   |                              // Must support double buffering
          PFD_SWAP_EXCHANGE ),                              // Hint to the driver to exchange the back- with the front buffer
        PFD_TYPE_RGBA,                                      // Request an RGBA format
        videoMode_.colorBitDepth,       /* cColorBits */    // Select color bit depth
        0, 0, 0, 0, 0, 0,                                   // Color bits ignored
        8,                              /* cAlphaBits */    // Request an alpha buffer (8 Bits)
        0,                                                  // Shift bit ignored
        0,                                                  // No accumulation buffer
        0, 0, 0, 0,                                         // Accumulation bits ignored
        24,                             /* cDepthBits */    // Z-Buffer bits (Depth Buffer)
        1,                              /* cStencilBits */  // Stencil buffer
        0,                                                  // No auxiliary buffer
        0,                                                  // Main drawing layer (No longer used)
        0,                                                  // Reserved
        0, 0, 0                                             // Layer masks ignored
    };
    
    /* Try to find suitable pixel format */
    const bool wantAntiAliasFormat = (contextDesc_.antiAliasingDesc.isEnabled && numMSPixelFormats_ > 0);

    size_t msPixelFormatIndex = 0;
    bool wasStandardFormatUsed = false;

    while (1)
    {
        if (wantAntiAliasFormat && msPixelFormatIndex < GLRenderContext::numMaxMSPixelFormats)
        {
            /* Choose anti-aliasing pixel format */
            pixelFormat_ = multiSamplePixelFormats_[msPixelFormatIndex++];
        }
        
        if (!pixelFormat_)
        {
            /* Choose standard pixel format */
            pixelFormat_ = ChoosePixelFormat(deviceContext_, &formatDesc);
            
            if (wantAntiAliasFormat)
                IO::Log::Error(errAntiAliasingNotSupported);
            
            wasStandardFormatUsed = true;
        }
        
        /* Check for errors */
        if (!pixelFormat_)
            throw RenderContextException("Pixel format selection failed");
        
        /* Set pixel format */
        auto wasFormatSelected = SetPixelFormat(deviceContext_, pixelFormat_, &formatDesc);
        
        if (!wasFormatSelected)
        {
            if (wasStandardFormatUsed)
                throw RenderContextException("Setting up pixel format failed");
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
    auto& antiAliasDesc = contextDesc_.antiAliasingDesc;

    /*
    Load GL extension "wglChoosePixelFormatARB" to choose anti-aliasing pixel formats
    A valid (standard) GL context must be created at this time, before an extension can be loaded!
    */
    if (!wglChoosePixelFormatARB && !GLExtensionLoader::LoadPixelFormatProcs())
    {
        ClearMSPixelFormats();
        return false;
    }

    /* Setup pixel format for anti-aliasing */
    const auto queriedMultiSamples = antiAliasDesc.multiSamples;

    while (antiAliasDesc.multiSamples > 0)
    {
        numMSPixelFormats_ = 0;
        float attribsFlt[] = { 0.0f, 0.0f };

        int attribsInt[] =
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
            WGL_COLOR_BITS_ARB,     videoMode_.colorBitDepth,
            WGL_ALPHA_BITS_ARB,     8,
            WGL_DEPTH_BITS_ARB,     24,
            WGL_STENCIL_BITS_ARB,   1,
            WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
            WGL_SAMPLE_BUFFERS_ARB, (antiAliasDesc.isEnabled ? GL_TRUE : GL_FALSE),
            WGL_SAMPLES_ARB,        antiAliasDesc.multiSamples,
            0, 0
        };

        /* Choose new pixel format with anti-aliasing */
        int result = wglChoosePixelFormatARB(
            deviceContext_,
            attribsInt,
            attribsFlt,
            GLRenderContext::numMaxMSPixelFormats,
            multiSamplePixelFormats_,
            &numMSPixelFormats_
        );

        if (!result || numMSPixelFormats_ < 1)
        {
            if (antiAliasDesc.multiSamples <= 0)
            {
                /* Lowest count of multi-samples reached -> return with error */
                return false;
            }

            /* Choose next lower count of multi-samples */
            --antiAliasDesc.multiSamples;
        }
        else
        {
            /* Found suitable pixel formats */
            break;
        }
    }

    /* Check if multi-sample count was reduced */
    if (antiAliasDesc.multiSamples < queriedMultiSamples)
    {
        IO::Log::Warning(
            "Reduced multi-samples for anti-aliasing from " +
            ToStr(queriedMultiSamples) + " to " + ToStr(antiAliasDesc.multiSamples)
        );
    }

    /* Enable anti-aliasing */
    GLSetState(GL_MULTISAMPLE, antiAliasDesc.isEnabled);

    return true;
}

void GLRenderContext::CopyPixelFormat(GLRenderContext* sourceContext)
{
    if (sourceContext)
    {
        pixelFormat_ = sourceContext->pixelFormat_;
        std::copy(
            sourceContext->multiSamplePixelFormats_,
            sourceContext->multiSamplePixelFormats_ + numMaxMSPixelFormats,
            multiSamplePixelFormats_
        );
        numMSPixelFormats_ = sourceContext->numMSPixelFormats_;
    }
}

bool GLRenderContext::SetupVSyncInterval()
{
    /* Load GL extension "wglSwapIntervalEXT" to set v-sync interval */
    if (!wglSwapIntervalEXT && !GLExtensionLoader::LoadSwapIntervalProcs())
        return false;

    /* Setup v-sync interval */
    int interval = contextDesc_.vsyncDesc.isEnabled ? static_cast<int>(contextDesc_.vsyncDesc.interval) : 0;
    wglSwapIntervalEXT(interval);

    return true;
}

void GLRenderContext::ReCreateFrame()
{
    /* Inform the client programmer that the frame must be re-created */
    CallFrameReCreateCallback();

    /* Just create a new window with the same settings */
    CreateFrame(frame_->Title(), frame_->GetParentWindow());
}

void GLRenderContext::ReCreateFrameAndUpdatePixelFormat()
{
    /*
    For anti-aliasing we must recreate the window,
    because a pixel format can be choosen only once for a window in Win32.
    Then re-setup device context and pixel format
    */
    ReCreateFrame();
    SetupDeviceContextAndPixelFormat();
}

#endif


} // /namespace LLGL



// ================================================================================
