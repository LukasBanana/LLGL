/*
 * WasmGLContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WasmGLContext.h"
#include "../../../CheckedCast.h"
#include "../../../StaticAssertions.h"
#include "../../../../Core/CoreUtils.h"
#include "../../../../Core/Exception.h"
#include <LLGL/RendererConfiguration.h>
#include <LLGL/Backend/OpenGL/NativeHandle.h>


namespace LLGL
{


/*
 * GLContext class
 */

LLGL_ASSERT_STDLAYOUT_STRUCT( OpenGL::RenderSystemNativeHandle );

std::unique_ptr<GLContext> GLContext::Create(
    const GLPixelFormat&                pixelFormat,
    const RendererConfigurationOpenGL&  profile,
    Surface&                            surface,
    GLContext*                          sharedContext,
    const ArrayView<char>&              /*customNativeHandle*/)
{
    WasmGLContext* sharedContextEGL = (sharedContext != nullptr ? LLGL_CAST(WasmGLContext*, sharedContext) : nullptr);
    return MakeUnique<WasmGLContext>(pixelFormat, profile, surface, sharedContextEGL);
}


/*
 * WasmGLContext class
 */

WasmGLContext::WasmGLContext(
    const GLPixelFormat&                pixelFormat,
    const RendererConfigurationOpenGL&  profile,
    Surface&                            surface,
    WasmGLContext*                      sharedContext)
{
    CreateContext(pixelFormat, profile, sharedContext);
}

WasmGLContext::~WasmGLContext()
{
    DeleteContext();
}

int WasmGLContext::GetSamples() const
{
    return samples_;
}

bool WasmGLContext::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(OpenGL::RenderSystemNativeHandle))
    {
        auto* nativeHandleGL = static_cast<OpenGL::RenderSystemNativeHandle*>(nativeHandle);
        nativeHandleGL->context = webGLContextHandle_;
        return true;
    }
    return false;
}


/*
 * ======= Private: =======
 */

bool WasmGLContext::SetSwapInterval(int /*interval*/)
{
    return false; // dummy
}

static void GetWebGLVersionFromConfig(EmscriptenWebGLContextAttributes& attrs, const RendererConfigurationOpenGL& cfg)
{
    if (cfg.majorVersion == 0 && cfg.minorVersion == 0)
    {
        /* WebGL 2.0 is requested by default */
        attrs.majorVersion = 2;
        attrs.minorVersion = 0;
    }
    else
    {
        /* Request custom WebGL version (can only be 1.0 or 2.0) */
        attrs.majorVersion = cfg.majorVersion;
        attrs.minorVersion = cfg.minorVersion;
    }
}

// Run JavaScript code to detect Safari user agent. This is generally not advised because it's not future proof,
// but there is currently no other way to detect whether WebGL 2 is properly supported or not. It is not on Safari sadly.
EM_JS(int, EmscriptenIsSafariUserAgent, (void), {
    return (navigator.userAgent.includes('Safari') && !navigator.userAgent.includes('Chrome') ? 1 : 0);
})

// Hacky function to determine whether we can trust the browser engine to properly support the WebGL translation layer.
static bool IsWebGLSwapControlWorkaroundRequired()
{
    return (EmscriptenIsSafariUserAgent() != 0);
}

void WasmGLContext::CreateContext(const GLPixelFormat& pixelFormat, const RendererConfigurationOpenGL& profile, WasmGLContext* sharedContext)
{
    /*
    With WebGL, we assume that the maximum sample count is 4.
    When the swap-control workaround is rquired, we enable anti-aliasing and swap-control.
    Otherwise, synchronization issues with glBufferSubData() can be observed, likely caused by the translation layer (on both macOS and iOS Safari).
    */
    if (IsWebGLSwapControlWorkaroundRequired())
    {
        samples_ = 4;
        hasExplicitSwapControl_ = true;
    }
    else
    {
        samples_ = Clamp(pixelFormat.samples, 1, 4);
        hasExplicitSwapControl_ = false;
    }

    /* Initialiye WebGL context attributes; Default to WebGL 2.0 */
	EmscriptenWebGLContextAttributes attrs = {};
	emscripten_webgl_init_context_attributes(&attrs);

    GetWebGLVersionFromConfig(attrs, profile);
	attrs.alpha                         = true;
	attrs.depth                         = true;
	attrs.stencil                       = true;
	attrs.antialias                     = (samples_ > 1);
	attrs.premultipliedAlpha            = false; // This must be disabled to prevent glitches in the browser canvas
	attrs.enableExtensionsByDefault     = true;
	attrs.powerPreference               = EM_WEBGL_POWER_PREFERENCE_DEFAULT;

    /* If explicit swap-control is requested, offscreen back-buffering must also be enabled */
    if (hasExplicitSwapControl_)
    {
        attrs.explicitSwapControl           = true;
        attrs.renderViaOffscreenBackBuffer  = true;
    }

    /* Create WebGL context */
    //TODO: determine canvas ID
	webGLContextHandle_ = emscripten_webgl_create_context("#canvas", &attrs);
    if (!webGLContextHandle_)
        LLGL_TRAP("emscripten_webgl_create_context() failed");

    /* Make WebGL context current */
    EMSCRIPTEN_RESULT result = emscripten_webgl_make_context_current(webGLContextHandle_);
    LLGL_ASSERT(result == EMSCRIPTEN_RESULT_SUCCESS, "emscripten_webgl_make_context_current() failed");
}

void WasmGLContext::DeleteContext()
{
    EMSCRIPTEN_RESULT result = emscripten_webgl_destroy_context(webGLContextHandle_);
    LLGL_ASSERT(result == EMSCRIPTEN_RESULT_SUCCESS, "emscripten_webgl_destroy_context() failed");
    webGLContextHandle_ = 0;
}


} // /namespace LLGL



// ================================================================================
