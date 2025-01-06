/*
 * Win32GLContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <Windows.h>

#include "Win32GLContext.h"
#include "../../Ext/GLExtensions.h"
#include "../../Ext/GLExtensionLoader.h"
#include "../../../CheckedCast.h"
#include "../../../StaticAssertions.h"
#include "../../../RenderSystemUtils.h"
#include "../../../TextureUtils.h"
#include "../../../../Core/CoreUtils.h"
#include "../../../../Core/Assertion.h"
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Backend/OpenGL/NativeHandle.h>
#include <LLGL/Log.h>
#include <algorithm>


namespace LLGL
{


/*
 * WGLProxyWindowClass struct
 */

struct WGLProxyWindowClass
{
    static const TCHAR* GetName()
    {
        return TEXT("LLGL.WGLProxyWindowClass");
    }

    WGLProxyWindowClass()
    {
        /* Setup window class information */
        WNDCLASS wc = {};

        wc.style            = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.hInstance        = GetModuleHandle(nullptr);
        wc.lpfnWndProc      = DefWindowProc;
        wc.hIcon            = nullptr;
        wc.hCursor          = nullptr;
        wc.hbrBackground    = nullptr;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;
        wc.lpszMenuName     = nullptr;
        wc.lpszClassName    = GetName();

        /* Register window class */
        if (!RegisterClass(&wc))
            LLGL_TRAP("failed to register window class");
    }

    ~WGLProxyWindowClass()
    {
        UnregisterClass(GetName(), GetModuleHandle(nullptr));
    }
};

static std::unique_ptr<WGLProxyWindowClass> g_wglProxyWindowClass;

static const TCHAR* GetProxyWindowClassName()
{
    /* Register Win32 window class if not already done */
    if (!g_wglProxyWindowClass)
        g_wglProxyWindowClass = MakeUnique<WGLProxyWindowClass>();
    return WGLProxyWindowClass::GetName();
}


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
    Win32GLContext* sharedContextWGL = (sharedContext != nullptr ? LLGL_CAST(Win32GLContext*, sharedContext) : nullptr);
    return MakeUnique<Win32GLContext>(
        pixelFormat, profile, surface, sharedContextWGL,
        GetRendererNativeHandle<OpenGL::RenderSystemNativeHandle>(customNativeHandle)
    );
}


/*
 * Win32GLContext class
 */

static void DeleteWGLContext(HGLRC& hGLRC)
{
    if (wglDeleteContext(hGLRC) == FALSE)
        Log::Errorf("wglDeleteContext failed");
    hGLRC = nullptr;
}

static bool MakeWGLContextCurrent(HDC hDC, HGLRC hGLRC)
{
    if (wglMakeCurrent(hDC, hGLRC) == FALSE)
    {
        Log::Errorf(
            "wglMakeCurrent((HDC)%p, (HGLRC)%p) failed",
            reinterpret_cast<const void*>(hDC), reinterpret_cast<const void*>(hGLRC)
        );
        return false;
    }
    return true;
}

Win32GLContext::Win32GLContext(
    const GLPixelFormat&                    pixelFormat,
    const RendererConfigurationOpenGL&      profile,
    Surface&                                surface,
    Win32GLContext*                         sharedContext,
    const OpenGL::RenderSystemNativeHandle* customNativeHandle)
:
    profile_     { profile                       },
    formatDesc_  { pixelFormat                   },
    isProxyGLRC_ { customNativeHandle != nullptr }
{
    if (isProxyGLRC_)
    {
        /* Create a proxy context which only caches the provided WGL context and its pixel format */
        CreateProxyContext(surface, *customNativeHandle);
    }
    else if (sharedContext)
    {
        /* Create WGL context with a shared GL context */
        Win32GLContext* sharedContextWGL = LLGL_CAST(Win32GLContext*, sharedContext);
        CreateWGLContext(surface, sharedContextWGL);
    }
    else
    {
        /* Create default WGL context */
        CreateWGLContext(surface);
    }
}

Win32GLContext::~Win32GLContext()
{
    /* Only delete this WGL context if we own it. A proxy context does not own the WGL context as it was provided externally. */
    if (!isProxyGLRC_)
        DeleteWGLContext(hGLRC_);
}

int Win32GLContext::GetSamples() const
{
    return formatDesc_.samples;
}

bool Win32GLContext::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const
{
    if (auto* nativeHandleGL = GetTypedNativeHandle<OpenGL::RenderSystemNativeHandle>(nativeHandle, nativeHandleSize))
    {
        nativeHandleGL->context = hGLRC_;
        return true;
    }
    return false;
}


/*
 * ======= Private: =======
 */

bool Win32GLContext::SetSwapInterval(int interval)
{
    /* Load GL extension "wglSwapIntervalEXT" to set swap interval */
    if (wglSwapIntervalEXT == nullptr)
        LoadSwapIntervalProcs();
    if (wglSwapIntervalEXT == nullptr)
        return false;
    return (wglSwapIntervalEXT(interval) == TRUE);
}

static HDC GetWin32DeviceContext(Surface& surface)
{
    /* Get device context from native window */
    NativeHandle nativeHandle = {};
    if (!surface.GetNativeHandle(&nativeHandle, sizeof(nativeHandle)))
        LLGL_TRAP("invalid native Win32 window handle");
    return ::GetDC(nativeHandle.window);
}

void Win32GLContext::CreateProxyContext(Surface& surface, const OpenGL::RenderSystemNativeHandle& nativeHandle)
{
    LLGL_ASSERT_PTR(nativeHandle.context);

    /* Get the surface's Win32 device context and choose pixel format */
    hDC_ = GetWin32DeviceContext(surface);
    SelectPixelFormat(hDC_);

    /* Store custom native handle */
    hGLRC_ = nativeHandle.context;

    if (!MakeWGLContextCurrent(hDC_, hGLRC_))
        LLGL_TRAP("failed to make initial GL context current");
}

static HWND CreateProxyWindow()
{
    return CreateWindow(
        GetProxyWindowClassName(),
        TEXT("LLGL.Win32GLContext.ProxyWindow"),
        WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        nullptr,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );
}

void Win32GLContext::CreateWGLContext(Surface& surface, Win32GLContext* sharedContext)
{
    const bool hasMultiSampling = (formatDesc_.samples > 1);

    /* Is multi-sampling requested but no suitable pixel format is cached yet? */
    if (hasMultiSampling && pixelFormatsMSCount_ == 0)
    {
        /*
        A multi-sampling render context is created in these steps:
        1. Create a proxy Win32 window to get a valid device context (HDC).
        2. Create a default WGL context to get a valid OpenGL render context (HGLRC).
        3. Load the OpenGL extension procedure to select a multi-sample pixel format (wglChoosePixelFormatARB).
        4. Cache available multi-sample pixel formats.
        5. Delete proxy window.
        */
        HWND proxyWnd = CreateProxyWindow();
        LLGL_ASSERT(proxyWnd != nullptr);
        HDC proxyDC = GetDC(proxyWnd);
        SelectPixelFormat(proxyDC);
        HGLRC proxyGLRC = CreateStandardWGLContext(proxyDC);
        if (!SelectMultisampledPixelFormat(proxyDC))
            ErrorMultisampleContextFailed();
        DeleteWGLContext(proxyGLRC);
        ::DestroyWindow(proxyWnd);
    }

    /* Get the surface's Win32 device context */
    hDC_ = GetWin32DeviceContext(surface);

    /* If a shared context is specified, use its pre-selected pixel format */
    if (hasMultiSampling && sharedContext != nullptr && sharedContext->GetSamples() >= GetSamples())
        CopyPixelFormat(*sharedContext);

    /* First setup device context and choose pixel format */
    SelectPixelFormat(hDC_);

    /* Create standard render context first */
    hGLRC_ = CreateStandardWGLContext(hDC_);

    /* Check for extended render context */
    if (profile_.contextProfile != OpenGLContextProfile::CompatibilityProfile)
    {
        /*
        Load profile selection extension (wglCreateContextAttribsARB) via current context,
        then create new context with extended settings.
        */
        if (wglCreateContextAttribsARB || LoadCreateContextProcs())
        {
            if (HGLRC extRenderContext = CreateExplicitWGLContext(hDC_, sharedContext))
            {
                /* Use the extended profile and delete the old standard render context */
                DeleteWGLContext(hGLRC_);
                hGLRC_ = extRenderContext;
            }
            else
            {
                /* Print warning and disbale profile selection */
                Log::Errorf("failed to create extended OpenGL profile");
                profile_.contextProfile = OpenGLContextProfile::CompatibilityProfile;
            }
        }
        else
        {
            /* Print warning and disable profile settings */
            Log::Errorf("failed to select OpenGL profile");
            profile_.contextProfile = OpenGLContextProfile::CompatibilityProfile;
        }
    }

    /* Check if context creation was successful */
    if (!MakeWGLContextCurrent(hDC_, hGLRC_))
        LLGL_TRAP("failed to make initial GL context current");

    /* Share resources with previous render context (only for compatibility profile) */
    if (sharedContext && profile_.contextProfile == OpenGLContextProfile::CompatibilityProfile)
    {
        if (!wglShareLists(sharedContext->hGLRC_, hGLRC_))
        {
            LLGL_TRAP(
                "wglShareLists((HGLRC)%p, (HGLRC)%p) failed",
                reinterpret_cast<const void*>(sharedContext->hGLRC_), reinterpret_cast<const void*>(hGLRC_)
            );
        }
    }
}

HGLRC Win32GLContext::CreateStandardWGLContext(HDC hDC)
{
    /* Create OpenGL "Compatibility Profile" render context */
    HGLRC hGLRC = wglCreateContext(hDC);

    if (!hGLRC)
        LLGL_TRAP("wglCreateContext failed");

    /* Make GL context current or delete context on failure */
    if (!MakeWGLContextCurrent(hDC, hGLRC))
    {
        DeleteWGLContext(hGLRC);
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
        Log::Errorf("invalid version for OpenGL profile");
        return nullptr;
    }
    else if (error == ERROR_INVALID_PROFILE_ARB)
    {
        Log::Errorf("invalid OpenGL profile");
        return nullptr;
    }

    /* Make GL context current or delete context on failure */
    if (!MakeWGLContextCurrent(hDC, hGLRC))
    {
        DeleteWGLContext(hGLRC);
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

bool Win32GLContext::SelectPixelFormat(HDC hDC)
{
    /* Setup pixel format attributes */
    PIXELFORMATDESCRIPTOR formatDesc;
    GetWGLPixelFormatDesc(formatDesc_, formatDesc);

    /* Try to find suitable pixel format */
    const bool isMultisampleFormatRequested = (formatDesc_.samples > 1 && pixelFormatsMSCount_ > 0);

    bool wasStandardFormatUsed = false;

    for (UINT pixelFormatMSIndex = 0;;)
    {
        if (isMultisampleFormatRequested && pixelFormatMSIndex < pixelFormatsMSCount_)
        {
            /* Choose multi-sample pixel format */
            pixelFormat_ = pixelFormatsMS_[pixelFormatMSIndex++];
        }

        if (!pixelFormat_)
        {
            /* Choose standard pixel format */
            pixelFormat_ = ::ChoosePixelFormat(hDC, &formatDesc);

            if (isMultisampleFormatRequested)
            {
                ErrorMultisampleContextFailed();
                return false;
            }

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
            LLGL_TRAP("failed to select pixel format");

        /* Set pixel format */
        const BOOL wasFormatSelected = ::SetPixelFormat(hDC, pixelFormat_, &formatDesc);
        if (!wasFormatSelected)
        {
            if (wasStandardFormatUsed)
                LLGL_TRAP("failed to set default pixel format");
            if (pixelFormatMSIndex == pixelFormatsMSCount_)
            {
                ErrorMultisampleContextFailed();
                return false;
            }
        }
        else
        {
            /* Format was selected -> quit with success */
            break;
        }
    }

    return true;
}

bool Win32GLContext::SelectMultisampledPixelFormat(HDC hDC)
{
    /*
    Load GL extension "wglChoosePixelFormatARB" to choose multi-sample pixel formats
    A valid (standard) GL context must be created at this time, before an extension can be loaded!
    */
    if (wglChoosePixelFormatARB == nullptr)
    {
        if (!LoadPixelFormatProcs() || wglChoosePixelFormatARB == nullptr)
            return false;
    }

    const float attribsFlt[] = { 0.0f, 0.0f };

    /* Reduce sample count successively if we fail to select a pixel format with the current sample count */
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

        /* Choose new pixel format with current number of samples */
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

void Win32GLContext::ErrorMultisampleContextFailed()
{
    /* Print error and disable multi-sampled context */
    Log::Errorf("multi-sampled OpenGL context is not supported");
    formatDesc_.samples = 1;
}


} // /namespace LLGL



// ================================================================================
