/*
 * EmscriptenGLContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "EmscriptenGLContext.h"
#include "../../../CheckedCast.h"
#include "../../../StaticAssertions.h"
#include "../../../../Core/CoreUtils.h"
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
    EmscriptenGLContext* sharedContextEGL = (sharedContext != nullptr ? LLGL_CAST(EmscriptenGLContext*, sharedContext) : nullptr);
    return MakeUnique<EmscriptenGLContext>(pixelFormat, profile, surface, sharedContextEGL);
}


/*
 * EmscriptenGLContext class
 */

EmscriptenGLContext::EmscriptenGLContext(
    const GLPixelFormat&                pixelFormat,
    const RendererConfigurationOpenGL&  profile,
    Surface&                            surface,
    EmscriptenGLContext*                sharedContext)
{
    CreateContext(pixelFormat, profile, sharedContext);
}

EmscriptenGLContext::~EmscriptenGLContext()
{
    DeleteContext();
}

int EmscriptenGLContext::GetSamples() const
{
    int samples_ = 4;
    return samples_;
}

bool EmscriptenGLContext::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const
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

bool EmscriptenGLContext::SetSwapInterval(int interval)
{
    return true;
}

void EmscriptenGLContext::CreateContext(const GLPixelFormat& pixelFormat, const RendererConfigurationOpenGL& profile, EmscriptenGLContext* sharedContext)
{
	EmscriptenWebGLContextAttributes attrs;
	emscripten_webgl_init_context_attributes(&attrs);
	attrs.majorVersion = 2;
	attrs.minorVersion = 0;
	attrs.alpha = false;
	attrs.depth = false;
	attrs.stencil = false;
	attrs.antialias = false;
	attrs.premultipliedAlpha = true;
	attrs.preserveDrawingBuffer = false;
	attrs.explicitSwapControl = 0;
    attrs.enableExtensionsByDefault = true;
	//attrs.preferLowPowerToHighPerformance = false;
	attrs.failIfMajorPerformanceCaveat = false;
	attrs.enableExtensionsByDefault = true;


	context_ = emscripten_webgl_create_context("#mycanvas", &attrs);

    if (!context_)
        throw std::runtime_error("emscripten_webgl_create_context failed");

    EMSCRIPTEN_RESULT res = emscripten_webgl_make_context_current(context_);
}

void EmscriptenGLContext::DeleteContext()
{
    //destroy context_
}


} // /namespace LLGL



// ================================================================================
