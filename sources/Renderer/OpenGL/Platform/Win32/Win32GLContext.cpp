/*
 * Win32GLContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Win32GLContext.h"
#include "../../Ext/GLExtensions.h"
#include "../../Ext/GLExtensionLoader.h"
#include "../../../CheckedCast.h"
#include "../../../TextureUtils.h"
#include "../../../../Core/Helper.h"
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Log.h>
#include <algorithm>


namespace LLGL
{


/*
 * GLContext class
 */

std::unique_ptr<GLContext> GLContext::Create(
    const RenderContextDescriptor&      desc,
    const RendererConfigurationOpenGL&  config,
    Surface&                            surface,
    GLContext*                          sharedContext)
{
    Win32GLContext* sharedContextWGL = (sharedContext != nullptr ? LLGL_CAST(Win32GLContext*, sharedContext) : nullptr);
    return MakeUnique<Win32GLContext>(desc, config, surface, sharedContextWGL);
}


/*
 * Win32GLContext class
 */

Win32GLContext::Win32GLContext(
    const RenderContextDescriptor&      desc,
    const RendererConfigurationOpenGL&  config,
    Surface&                            surface,
    Win32GLContext*                     sharedContext)
:
    GLContext { sharedContext                                     },
    samples_  { static_cast<int>(GetClampedSamples(desc.samples)) },
    surface_  { surface                                           }
{
    /* Initialize context parameters */
    WGLContextParams contextParams;
    {
        contextParams.profile       = config;
        contextParams.colorBits     = desc.videoMode.colorBits;
        contextParams.depthBits     = desc.videoMode.depthBits;
        contextParams.stencilBits   = desc.videoMode.stencilBits;
    }

    /* Create WGL context */
    if (sharedContext)
    {
        auto sharedContextWGL = LLGL_CAST(Win32GLContext*, sharedContext);
        CreateContext(contextParams, sharedContextWGL);
    }
    else
        CreateContext(contextParams);
}

Win32GLContext::~Win32GLContext()
{
    DeleteContext();
}

bool Win32GLContext::SetSwapInterval(int interval)
{
    /* Load GL extension "wglSwapIntervalEXT" to set swap interval */
    if (wglSwapIntervalEXT || LoadSwapIntervalProcs())
        return (wglSwapIntervalEXT(interval) == TRUE);
    else
        return false;
}

bool Win32GLContext::SwapBuffers()
{
    return (::SwapBuffers(hDC_) == TRUE);
}

void Win32GLContext::Resize(const Extent2D& resolution)
{
    // do nothing (WGL context does not need to be resized)
}

std::uint32_t Win32GLContext::GetSamples() const
{
    return samples_;
}


/*
 * ======= Private: =======
 */

bool Win32GLContext::Activate(bool activate)
{
    if (activate)
        return (wglMakeCurrent(hDC_, hGLRC_) == TRUE);
    else
        return (wglMakeCurrent(0, 0) == TRUE);
}

static void ErrAntiAliasingNotSupported()
{
    Log::PostReport(Log::ReportType::Error, "multi-sample anti-aliasing is not supported");
}

/*
TODO:
- When anti-aliasing and extended-profile-selection is enabled,
  maximal 2 contexts should be created (and not 3).
*/
void Win32GLContext::CreateContext(const WGLContextParams& params, Win32GLContext* sharedContext)
{
    const bool hasMultiSampling = (samples_ > 1);

    /* If a shared context has passed, use its pre-selected pixel format */
    if (hasMultiSampling && sharedContext)
        CopyPixelFormat(*sharedContext);

    /* First setup device context and choose pixel format */
    SetupDeviceContextAndPixelFormat(params);

    /* Create standard render context first */
    auto stdRenderContext = CreateGLContext(params, false, sharedContext);

    if (!stdRenderContext)
        throw std::runtime_error("failed to create standard OpenGL render context");

    /* Check for multi-sample anti-aliasing */
    if (hasMultiSampling && !hasSharedContext_)
    {
        /* Setup anti-aliasing after creating a standard render context. */
        if (SetupAntiAliasing(params))
        {
            /* Delete old standard render context */
            DeleteGLContext(stdRenderContext);

            /*
            For anti-aliasing we must recreate the window,
            because a pixel format can be choosen only once for a Win32 window,
            then update device context and pixel format
            */
            RecreateWindow(params);

            /* Create a new render context -> now with anti-aliasing pixel format */
            stdRenderContext = CreateGLContext(params, false, sharedContext);

            if (!stdRenderContext)
                Log::PostReport(Log::ReportType::Error, "failed to create multi-sample anti-aliasing");
        }
        else
        {
            /* Print warning and disable anti-aliasing */
            ErrAntiAliasingNotSupported();
            samples_ = 1;
        }
    }

    hGLRC_ = stdRenderContext;

    /* Check for extended render context */
    auto contextProfile = params.profile.contextProfile;
    if (contextProfile != OpenGLContextProfile::CompatibilityProfile && !hasSharedContext_)
    {
        /*
        Load profile selection extension (wglCreateContextAttribsARB) via current context,
        then create new context with extended settings.
        */
        if (wglCreateContextAttribsARB || LoadCreateContextProcs())
        {
            auto extRenderContext = CreateGLContext(params, true, sharedContext);

            if (extRenderContext)
            {
                /* Use the extended profile and delete the old standard render context */
                hGLRC_ = extRenderContext;
                DeleteGLContext(stdRenderContext);
            }
            else
            {
                /* Print warning and disbale profile selection */
                Log::PostReport(Log::ReportType::Error, "failed to create extended OpenGL profile");
                contextProfile = OpenGLContextProfile::CompatibilityProfile;
            }
        }
        else
        {
            /* Print warning and disable profile settings */
            Log::PostReport(Log::ReportType::Error, "failed to select OpenGL profile");
            contextProfile = OpenGLContextProfile::CompatibilityProfile;
        }
    }

    /* Check if context creation was successful */
    if (!hGLRC_)
        throw std::runtime_error("failed to create OpenGL render context");

    if (wglMakeCurrent(hDC_, hGLRC_) != TRUE)
        throw std::runtime_error("failed to activate OpenGL render context");

    /*
    Share resources with previous render context (only for compatibility profile).
    -> Only do this, if this context has its own GL hardware context (hasSharedContext_ == false),
       but a shared render context was passed (sharedContext != null).
    */
    if (sharedContext && !hasSharedContext_ && contextProfile == OpenGLContextProfile::CompatibilityProfile)
    {
        if (!wglShareLists(sharedContext->hGLRC_, hGLRC_))
            throw std::runtime_error("failed to share resources from OpenGL render context");
    }

    /* Query GL version of final render context */
    //QueryGLVersion();
}

void Win32GLContext::DeleteContext()
{
    if (!hasSharedContext_)
    {
        /* Deactivate context before deletion */
        if (GLContext::Active() == this)
            Activate(false);

        DeleteGLContext(hGLRC_);
    }
}

void Win32GLContext::DeleteGLContext(HGLRC& renderContext)
{
    /* Delete GL render context */
    if (!wglDeleteContext(renderContext))
        Log::PostReport(Log::ReportType::Error, "failed to delete OpenGL render context");
    else
        renderContext = 0;
}

HGLRC Win32GLContext::CreateGLContext(const WGLContextParams& params, bool useExtProfile, Win32GLContext* sharedContext)
{
    /* Create hardware render context */
    HGLRC renderContext = 0;

    if (!sharedContext || !sharedContext->hGLRC_)
    {
        /* Create own hardware context */
        hasSharedContext_ = false;
        if (useExtProfile)
            renderContext = CreateExtContextProfile(params, sharedContext != nullptr ? sharedContext->hGLRC_ : nullptr);
        else
            renderContext = CreateStdContextProfile();
    }
    else
    {
        /* Use shared render context */
        hasSharedContext_ = true;
        renderContext = sharedContext->hGLRC_;
    }

    if (!renderContext)
        return 0;

    /* Activate new render context */
    if (wglMakeCurrent(hDC_, renderContext) != TRUE)
    {
        /* Print error and delete unusable render context */
        Log::PostReport(Log::ReportType::Error, "failed to active OpenGL render context (wglMakeCurrent)");
        DeleteGLContext(renderContext);
        return 0;
    }

    /* Query GL version of current render context */
    //QueryGLVersion();

    return renderContext;
}

HGLRC Win32GLContext::CreateStdContextProfile()
{
    /* Create OpenGL "Compatibility Profile" render context */
    return wglCreateContext(hDC_);
}

static int GLContextProfileToBitmask(const OpenGLContextProfile profile)
{
    switch (profile)
    {
        case OpenGLContextProfile::CompatibilityProfile:    return WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
        case OpenGLContextProfile::CoreProfile:             return WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
        #ifdef WGL_EXT_create_context_es_profile
        case OpenGLContextProfile::ESProfile:               return WGL_CONTEXT_ES_PROFILE_BIT_EXT;
        #endif
        default:                                            return WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
    }
}

HGLRC Win32GLContext::CreateExtContextProfile(const WGLContextParams& params, HGLRC sharedGLRC)
{
    /* Check if highest version possible shall be used */
    int major = params.profile.majorVersion;
    int minor = params.profile.minorVersion;

    if (major == 0 && minor == 0)
    {
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
    }

    /* Set up context flags */
    int contextFlags = 0;

    #ifdef LLGL_DEBUG
    contextFlags |= WGL_CONTEXT_DEBUG_BIT_ARB;
    #endif

    /* Set up extended attributes to select the OpenGL profile */
    const int attribList[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB,  major,
        WGL_CONTEXT_MINOR_VERSION_ARB,  minor,
        WGL_CONTEXT_FLAGS_ARB,          contextFlags,
        WGL_CONTEXT_PROFILE_MASK_ARB,   GLContextProfileToBitmask(params.profile.contextProfile),
        0, 0
    };

    /* Create OpenGL "Core Profile" or "Compatibility Profile" render context */
    HGLRC renderContext = wglCreateContextAttribsARB(hDC_, sharedGLRC, attribList);

    /* Check for errors */
    DWORD error = GetLastError();

    if (error == ERROR_INVALID_VERSION_ARB)
        Log::PostReport(Log::ReportType::Error, "invalid version for OpenGL profile");
    else if (error == ERROR_INVALID_PROFILE_ARB)
        Log::PostReport(Log::ReportType::Error, "invalid OpenGL profile");
    else
        return renderContext;

    return 0;
}

void Win32GLContext::SetupDeviceContextAndPixelFormat(const WGLContextParams& params)
{
    /* Get native window handle */
    NativeHandle nativeHandle = {};
    surface_.GetNativeHandle(&nativeHandle, sizeof(nativeHandle));

    if (!nativeHandle.window)
        throw std::runtime_error("invalid native Win32 window handle");

    /* Get device context from window */
    hDC_ = GetDC(nativeHandle.window);

    /* Select suitable pixel format */
    SelectPixelFormat(params);
}

void Win32GLContext::SelectPixelFormat(const WGLContextParams& params)
{
    /* Setup pixel format attributes */
    PIXELFORMATDESCRIPTOR formatDesc;
    {
        formatDesc.nSize            = sizeof(PIXELFORMATDESCRIPTOR);            // Structure size
        formatDesc.nVersion         = 1;                                        // Version number
        formatDesc.dwFlags          =
        (
            PFD_DRAW_TO_WINDOW |                                                // Format must support draw-to-window
            PFD_SUPPORT_OPENGL |                                                // Format must support OpenGL
            PFD_DOUBLEBUFFER   |                                                // Must support double buffering
            PFD_SWAP_EXCHANGE                                                   // Hint to the driver to exchange the back- with the front buffer
        );
        formatDesc.iPixelType       = PFD_TYPE_RGBA;                            // Request an RGBA format
        formatDesc.cColorBits       = static_cast<BYTE>(params.colorBits);      // Select color bit depth
        formatDesc.cRedBits         = 0;
        formatDesc.cRedShift        = 0;
        formatDesc.cGreenBits       = 0;
        formatDesc.cGreenShift      = 0;
        formatDesc.cBlueBits        = 0;
        formatDesc.cBlueShift       = 0;
        formatDesc.cAlphaBits       = (params.colorBits == 32 ? 8 : 0);         // Request an alpha buffer of 8 bits
        formatDesc.cAlphaShift      = 0;
        formatDesc.cAccumBits       = 0;                                        // No accumulation buffer
        formatDesc.cAccumRedBits    = 0;
        formatDesc.cAccumGreenBits  = 0;
        formatDesc.cAccumBlueBits   = 0;
        formatDesc.cAccumAlphaBits  = 0;
        formatDesc.cDepthBits       = static_cast<BYTE>(params.depthBits);      // Z-Buffer bits
        formatDesc.cStencilBits     = static_cast<BYTE>(params.stencilBits);    // Stencil buffer bits
        formatDesc.cAuxBuffers      = 0;                                        // No auxiliary buffer
        formatDesc.iLayerType       = 0;                                        // Main drawing layer (No longer used)
        formatDesc.bReserved        = 0;
        formatDesc.dwLayerMask      = 0;
        formatDesc.dwVisibleMask    = 0;
        formatDesc.dwDamageMask     = 0;
    }

    /* Try to find suitable pixel format */
    const bool isMultiSampleFormatRequested = (samples_ > 1 && pixelFormatsMSCount_ > 0);

    bool wasStandardFormatUsed = false;

    for (UINT pixelFormatMSIndex = 0;;)
    {
        if (isMultiSampleFormatRequested && pixelFormatMSIndex < Win32GLContext::maxPixelFormatsMS)
        {
            /* Choose anti-aliasing pixel format */
            pixelFormat_ = pixelFormatsMS_[pixelFormatMSIndex++];
        }

        if (!pixelFormat_)
        {
            /* Choose standard pixel format */
            pixelFormat_ = ChoosePixelFormat(hDC_, &formatDesc);

            if (isMultiSampleFormatRequested)
                ErrAntiAliasingNotSupported();

            wasStandardFormatUsed = true;
        }

        /* Check for errors */
        if (!pixelFormat_)
            throw std::runtime_error("failed to select pixel format");

        /* Set pixel format */
        auto wasFormatSelected = SetPixelFormat(hDC_, pixelFormat_, &formatDesc);

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

bool Win32GLContext::SetupAntiAliasing(const WGLContextParams& params)
{
    /*
    Load GL extension "wglChoosePixelFormatARB" to choose anti-aliasing pixel formats
    A valid (standard) GL context must be created at this time, before an extension can be loaded!
    */
    if (!wglChoosePixelFormatARB && !LoadPixelFormatProcs())
        return false;

    /* Setup pixel format for anti-aliasing */
    while (samples_ > 0)
    {
        const float attribsFlt[] = { 0.0f, 0.0f };

        const int attribsInt[] =
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
            WGL_COLOR_BITS_ARB,     24,
            WGL_ALPHA_BITS_ARB,     (params.colorBits == 32 ? 8 : 0),
            WGL_DEPTH_BITS_ARB,     params.depthBits,
            WGL_STENCIL_BITS_ARB,   params.stencilBits,
            WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
            WGL_SAMPLE_BUFFERS_ARB, (samples_ > 1 ? GL_TRUE : GL_FALSE),
            WGL_SAMPLES_ARB,        samples_,
            0, 0
        };

        /* Choose new pixel format with anti-aliasing */
        BOOL result = wglChoosePixelFormatARB(
            hDC_,
            attribsInt,
            attribsFlt,
            Win32GLContext::maxPixelFormatsMS,
            pixelFormatsMS_,
            &pixelFormatsMSCount_
        );

        if (!result || pixelFormatsMSCount_ < 1)
        {
            if (samples_ <= 0)
            {
                /* Lowest count of multi-samples reached -> return with error */
                return false;
            }

            /* Choose next lower count of multi-samples */
            --samples_;
        }
        else
        {
            /* Found suitable pixel formats */
            break;
        }
    }

    return true;
}

void Win32GLContext::CopyPixelFormat(Win32GLContext& sourceContext)
{
    /* Copy pixel format and array of multi-sampled pixel formats */
    pixelFormat_            = sourceContext.pixelFormat_;
    pixelFormatsMSCount_    = sourceContext.pixelFormatsMSCount_;
    ::memcpy(pixelFormatsMS_, sourceContext.pixelFormatsMS_, sizeof(pixelFormatsMS_));
}

void Win32GLContext::RecreateWindow(const WGLContextParams& params)
{
    /* Recreate window with current descriptor, then update device context and pixel format */
    surface_.ResetPixelFormat();
    SetupDeviceContextAndPixelFormat(params);
}


} // /namespace LLGL



// ================================================================================
