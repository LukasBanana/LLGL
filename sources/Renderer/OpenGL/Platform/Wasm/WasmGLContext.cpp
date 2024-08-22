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
    int samples_ = 4;
    return samples_;
}

bool WasmGLContext::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(OpenGL::RenderSystemNativeHandle))
    {
        auto* nativeHandleGL = reinterpret_cast<OpenGL::RenderSystemNativeHandle*>(nativeHandle);
        nativeHandleGL->context = context_;
        return true;
    }
    return false;
}


/*
 * ======= Private: =======
 */

bool WasmGLContext::SetSwapInterval(int interval)
{
    return true;
}

void WasmGLContext::CreateContext(const GLPixelFormat& pixelFormat, const RendererConfigurationOpenGL& profile, WasmGLContext* sharedContext)
{
	EmscriptenWebGLContextAttributes attrs;
	emscripten_webgl_init_context_attributes(&attrs);
	attrs.majorVersion                      = 2;
	attrs.minorVersion                      = 0;
	attrs.alpha                             = (pixelFormat.colorBits > 24);
	attrs.depth                             = (pixelFormat.depthBits > 0);
	attrs.stencil                           = (pixelFormat.stencilBits > 0);
	attrs.antialias                         = (pixelFormat.samples > 1);
	attrs.premultipliedAlpha                = true;
	attrs.preserveDrawingBuffer             = false;
	attrs.explicitSwapControl               = 0;
	attrs.failIfMajorPerformanceCaveat      = false;
	attrs.enableExtensionsByDefault         = true;
	attrs.powerPreference                   = EM_WEBGL_POWER_PREFERENCE_DEFAULT;

	context_ = emscripten_webgl_create_context("#canvas", &attrs);

    if (!context_)
        LLGL_TRAP("emscripten_webgl_create_context failed");

    EMSCRIPTEN_RESULT res = emscripten_webgl_make_context_current(context_);
}

void WasmGLContext::DeleteContext()
{
    //destroy context_
}


} // /namespace LLGL



// ================================================================================
