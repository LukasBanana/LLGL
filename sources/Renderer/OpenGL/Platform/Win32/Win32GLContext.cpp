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
#include "../../../../Core/CoreUtils.h"
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Log.h>
#include <algorithm>


namespace LLGL
{


static void DeleteGLContext(HGLRC& hGLRC)
{
    if (wglDeleteContext(hGLRC) == FALSE)
        Log::PostReport(Log::ReportType::Error, "wglDeleteContext failed");
    hGLRC = nullptr;
}

static bool MakeGLContextCurrent(HDC hDC, HGLRC hGLRC)
{
    if (wglMakeCurrent(hDC, hGLRC) == FALSE)
    {
        Log::PostReport(Log::ReportType::Error, "wglMakeCurrent failed");
        return false;
    }
    return true;
}


/*
 * GLContext class
 */

std::unique_ptr<GLContext> GLContext::Create(
    const GLPixelFormat&                pixelFormat,
    const RendererConfigurationOpenGL&  profile,
    Surface&                            surface,
    GLContext*                          sharedContext)
{
    Win32GLContext* sharedContextWGL = (sharedContext != nullptr ? LLGL_CAST(Win32GLContext*, sharedContext) : nullptr);
    return MakeUnique<Win32GLContext>(pixelFormat, profile, surface, sharedContextWGL);
}


/*
 * Win32GLContext class
 */

Win32GLContext::Win32GLContext(
    const GLPixelFormat&                pixelFormat,
    const RendererConfigurationOpenGL&  profile,
    Surface&                            surface,
    Win32GLContext*                     sharedContext)
:
    profile_    { profile     },
    formatDesc_ { pixelFormat }
{
    /* Create WGL context */
    if (sharedContext)
    {
        auto sharedContextWGL = LLGL_CAST(Win32GLContext*, sharedContext);
        CreateContext(surface, sharedContextWGL);
    }
    else
        CreateContext(surface);
}

Win32GLContext::~Win32GLContext()
{
    DeleteGLContext(hGLRC_);
}

void Win32GLContext::Resize(const Extent2D& resolution)
{
    // do nothing (WGL context does not need to be resized)
}

int Win32GLContext::GetSamples() const
{
    return formatDesc_.samples;
}


/*
 * ======= Private: =======
 */

bool Win32GLContext::SetSwapInterval(int interval)
{
    /* Load GL extension "wglSwapIntervalEXT" to set swap interval */
    if (wglSwapIntervalEXT || LoadSwapIntervalProcs())
        return (wglSwapIntervalEXT(interval) == TRUE);
    else
        return false;
}

static void ErrMultisampledGLContextNotSupported()
{
    Log::PostReport(Log::ReportType::Error, "multi-sampled OpenGL context is not supported");
}

static HDC GetWin32DeviceContext(const Surface& surface)
{
    /* Get device context from native window */
    NativeHandle nativeHandle = {};
    if (!surface.GetNativeHandle(&nativeHandle, sizeof(nativeHandle)))
        throw std::runtime_error("invalid native Win32 window handle");
    return ::GetDC(nativeHandle.window);
}

/*
TODO:
- When anti-aliasing and extended-profile-selection is enabled,
  maximal 2 contexts should be created (and not 3).
*/
void Win32GLContext::CreateContext(Surface& surface, Win32GLContext* sharedContext)
{
    const bool hasMultiSampling = (formatDesc_.samples > 1);

    /* Get the surface's Win32 device context */
    hDC_ = GetWin32DeviceContext(surface);

    /* If a shared context has passed, use its pre-selected pixel format */
    if (hasMultiSampling && sharedContext)
        CopyPixelFormat(*sharedContext);

    /* First setup device context and choose pixel format */
    SelectPixelFormat(surface);

    /* Create standard render context first */
    auto stdRenderContext = CreateStandardWGLContext(hDC_);

    /* Check for multi-sample anti-aliasing */
    if (hasMultiSampling)
    {
        /* Setup anti-aliasing after creating a standard render context. */
        if (SelectMultisampledPixelFormat(hDC_))
        {
            /* Delete old standard render context */
            DeleteGLContext(stdRenderContext);

            /*
            Update pixel format for the device context after a new pixel format has been selected.
            see https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-setpixelformat#remarks
            */
            hDC_ = UpdateSurfacePixelFormat(surface);

            /* Create a new render context -> now with anti-aliasing pixel format */
            stdRenderContext = CreateStandardWGLContext(hDC_);
        }
        else
        {
            /* Print warning and disable anti-aliasing */
            ErrMultisampledGLContextNotSupported();
            formatDesc_.samples = 1;
        }
    }

    hGLRC_ = stdRenderContext;

    /* Check for extended render context */
    if (profile_.contextProfile != OpenGLContextProfile::CompatibilityProfile)
    {
        /*
        Load profile selection extension (wglCreateContextAttribsARB) via current context,
        then create new context with extended settings.
        */
        if (wglCreateContextAttribsARB || LoadCreateContextProcs())
        {
            if (auto extRenderContext = CreateExplicitWGLContext(hDC_, sharedContext))
            {
                /* Use the extended profile and delete the old standard render context */
                hGLRC_ = extRenderContext;
                DeleteGLContext(stdRenderContext);
            }
            else
            {
                /* Print warning and disbale profile selection */
                Log::PostReport(Log::ReportType::Error, "failed to create extended OpenGL profile");
                profile_.contextProfile = OpenGLContextProfile::CompatibilityProfile;
            }
        }
        else
        {
            /* Print warning and disable profile settings */
            Log::PostReport(Log::ReportType::Error, "failed to select OpenGL profile");
            profile_.contextProfile = OpenGLContextProfile::CompatibilityProfile;
        }
    }

    /* Check if context creation was successful */
    if (wglMakeCurrent(hDC_, hGLRC_) != TRUE)
        throw std::runtime_error("wglMakeCurrent failed");

    /* Share resources with previous render context (only for compatibility profile) */
    if (sharedContext && profile_.contextProfile == OpenGLContextProfile::CompatibilityProfile)
    {
        if (!wglShareLists(sharedContext->hGLRC_, hGLRC_))
            throw std::runtime_error("wglShareLists failed");
    }
}

HGLRC Win32GLContext::CreateStandardWGLContext(HDC hDC)
{
    /* Create OpenGL "Compatibility Profile" render context */
    HGLRC hGLRC = wglCreateContext(hDC);

    if (!hGLRC)
        throw std::runtime_error("wglCreateContext failed");

    /* Make GL context current or delete context on failure */
    if (!MakeGLContextCurrent(hDC, hGLRC))
    {
        DeleteGLContext(hGLRC);
        return nullptr;
    }

    return hGLRC;
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

HGLRC Win32GLContext::CreateExplicitWGLContext(HDC hDC, Win32GLContext* sharedContext)
{
    /* Check if highest version possible shall be used */
    int major = profile_.majorVersion;
    int minor = profile_.minorVersion;

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
        WGL_CONTEXT_PROFILE_MASK_ARB,   GLContextProfileToBitmask(profile_.contextProfile),
        0, 0
    };

    /* Get shared WGL context */
    HGLRC sharedGLRC = (sharedContext != nullptr ? sharedContext->hGLRC_ : nullptr);

    /* Create OpenGL "Core Profile" or "Compatibility Profile" render context */
    HGLRC hGLRC = wglCreateContextAttribsARB(hDC, sharedGLRC, attribList);

    /* Check for errors */
    const DWORD error = GetLastError();

    if (error == ERROR_INVALID_VERSION_ARB)
    {
        Log::PostReport(Log::ReportType::Error, "invalid version for OpenGL profile");
        return nullptr;
    }
    else if (error == ERROR_INVALID_PROFILE_ARB)
    {
        Log::PostReport(Log::ReportType::Error, "invalid OpenGL profile");
        return nullptr;
    }

    /* Make GL context current or delete context on failure */
    if (!MakeGLContextCurrent(hDC, hGLRC))
    {
        DeleteGLContext(hGLRC);
        return nullptr;
    }

    return hGLRC;
}

static void GetWGLPixelFormatDesc(const GLPixelFormat& inDesc, PIXELFORMATDESCRIPTOR& outDesc)
{
    outDesc.nSize           = sizeof(PIXELFORMATDESCRIPTOR);            // Structure size
    outDesc.nVersion        = 1;                                        // Version number
    outDesc.dwFlags         =
    (
        PFD_DRAW_TO_WINDOW |                                            // Format must support draw-to-window
        PFD_SUPPORT_OPENGL |                                            // Format must support OpenGL
        PFD_DOUBLEBUFFER   |                                            // Must support double buffering
        PFD_SWAP_EXCHANGE                                               // Hint to the driver to exchange the back- with the front buffer
    );
    outDesc.iPixelType      = PFD_TYPE_RGBA;                            // Request an RGBA format
    outDesc.cColorBits      = static_cast<BYTE>(inDesc.colorBits);      // Select color bit depth
    outDesc.cRedBits        = 0;
    outDesc.cRedShift       = 0;
    outDesc.cGreenBits      = 0;
    outDesc.cGreenShift     = 0;
    outDesc.cBlueBits       = 0;
    outDesc.cBlueShift      = 0;
    outDesc.cAlphaBits      = (inDesc.colorBits == 32 ? 8 : 0);         // Request an alpha buffer of 8 bits
    outDesc.cAlphaShift     = 0;
    outDesc.cAccumBits      = 0;                                        // No accumulation buffer
    outDesc.cAccumRedBits   = 0;
    outDesc.cAccumGreenBits = 0;
    outDesc.cAccumBlueBits  = 0;
    outDesc.cAccumAlphaBits = 0;
    outDesc.cDepthBits      = static_cast<BYTE>(inDesc.depthBits);      // Z-Buffer bits
    outDesc.cStencilBits    = static_cast<BYTE>(inDesc.stencilBits);    // Stencil buffer bits
    outDesc.cAuxBuffers     = 0;                                        // No auxiliary buffer
    outDesc.iLayerType      = 0;                                        // Main drawing layer (No longer used)
    outDesc.bReserved       = 0;
    outDesc.dwLayerMask     = 0;
    outDesc.dwVisibleMask   = 0;
    outDesc.dwDamageMask    = 0;
}

void Win32GLContext::SelectPixelFormat(Surface& surface)
{
    /* Get the surface's Win32 device context */
    HDC hDC = GetWin32DeviceContext(surface);

    /* Setup pixel format attributes */
    PIXELFORMATDESCRIPTOR formatDesc;
    GetWGLPixelFormatDesc(formatDesc_, formatDesc);

    /* Try to find suitable pixel format */
    const bool isMultisampleFormatRequested = (formatDesc_.samples > 1 && pixelFormatsMSCount_ > 0);

    bool wasStandardFormatUsed = false;

    for (UINT pixelFormatMSIndex = 0;;)
    {
        if (isMultisampleFormatRequested && pixelFormatMSIndex < Win32GLContext::maxPixelFormatsMS)
        {
            /* Choose multi-sample pixel format */
            pixelFormat_ = pixelFormatsMS_[pixelFormatMSIndex++];
        }

        if (!pixelFormat_)
        {
            /* Choose standard pixel format */
            pixelFormat_ = ::ChoosePixelFormat(hDC, &formatDesc);

            if (isMultisampleFormatRequested)
                ErrMultisampledGLContextNotSupported();

            wasStandardFormatUsed = true;

            /* Deduce color and depth-stencil formats by pixel format descriptor */
            PIXELFORMATDESCRIPTOR selectedFormatDesc;
            ::DescribePixelFormat(hDC, pixelFormat_, sizeof(selectedFormatDesc), &selectedFormatDesc);
            DeduceColorFormat(
                selectedFormatDesc.cRedBits,
                selectedFormatDesc.cRedShift,
                selectedFormatDesc.cGreenBits,
                selectedFormatDesc.cGreenShift,
                selectedFormatDesc.cBlueBits,
                selectedFormatDesc.cBlueShift,
                selectedFormatDesc.cAlphaBits,
                selectedFormatDesc.cAlphaShift
            );
            DeduceDepthStencilFormat(
                selectedFormatDesc.cDepthBits,
                selectedFormatDesc.cStencilBits
            );
        }

        /* Check for errors */
        if (!pixelFormat_)
            throw std::runtime_error("failed to select pixel format");

        /* Set pixel format */
        const BOOL wasFormatSelected = ::SetPixelFormat(hDC, pixelFormat_, &formatDesc);
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

bool Win32GLContext::SelectMultisampledPixelFormat(HDC hDC)
{
    /*
    Load GL extension "wglChoosePixelFormatARB" to choose anti-aliasing pixel formats
    A valid (standard) GL context must be created at this time, before an extension can be loaded!
    */
    if (!wglChoosePixelFormatARB && !LoadPixelFormatProcs())
        return false;

    const float attribsFlt[] = { 0.0f, 0.0f };

    /* Setup pixel format for anti-aliasing */
    for (; formatDesc_.samples > 0; formatDesc_.samples--)
    {
        const int attribsInt[] =
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
            WGL_COLOR_BITS_ARB,     24,
            WGL_ALPHA_BITS_ARB,     (formatDesc_.colorBits == 32 ? 8 : 0),
            WGL_DEPTH_BITS_ARB,     formatDesc_.depthBits,
            WGL_STENCIL_BITS_ARB,   formatDesc_.stencilBits,
            WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
            WGL_SAMPLE_BUFFERS_ARB, (formatDesc_.samples > 1 ? GL_TRUE : GL_FALSE),
            WGL_SAMPLES_ARB,        formatDesc_.samples,
            0, 0
        };

        /* Choose new pixel format with anti-aliasing */
        const BOOL result = wglChoosePixelFormatARB(
            hDC,
            attribsInt,
            attribsFlt,
            Win32GLContext::maxPixelFormatsMS,
            pixelFormatsMS_,
            &pixelFormatsMSCount_
        );

        if (result && pixelFormatsMSCount_ > 0)
        {
            /* Found suitable pixel formats */
            return true;
        }
    }

    /* Lowest count of multi-samples reached -> return with error */
    return false;
}

void Win32GLContext::CopyPixelFormat(Win32GLContext& sourceContext)
{
    /* Copy pixel format and array of multi-sampled pixel formats */
    pixelFormat_            = sourceContext.pixelFormat_;
    pixelFormatsMSCount_    = sourceContext.pixelFormatsMSCount_;
    ::memcpy(pixelFormatsMS_, sourceContext.pixelFormatsMS_, sizeof(pixelFormatsMS_));
}

HDC Win32GLContext::UpdateSurfacePixelFormat(Surface& surface)
{
    /* Recreate window with current descriptor, then update device context and pixel format */
    surface.ResetPixelFormat();
    SelectPixelFormat(surface);
    return GetWin32DeviceContext(surface);
}


} // /namespace LLGL



// ================================================================================
