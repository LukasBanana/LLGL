/*
 * GLContextManager.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLContextManager.h"
#include "../RenderState/GLStateManager.h"
#include "../Ext/GLExtensionLoader.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../Profile/GLProfile.h"
#include "../../../Core/CoreUtils.h"
#include <LLGL/Window.h>
#include <LLGL/Canvas.h>
#include <cstring>


namespace LLGL
{


GLContextManager::GLContextManager(
    const RendererConfigurationOpenGL&  profile,
    const NewGLContextCallback&         newContextCallback,
    const void*                         customNativeHandle,
    std::size_t                         customNativeHandleSize)
:
    profile_            { profile            },
    newContextCallback_ { newContextCallback }
{
    /* Adjust context profile if Auto-selection is specified */
    if (profile_.contextProfile == OpenGLContextProfile::Auto)
        profile_.contextProfile = GLProfile::GetContextProfile();

    if (customNativeHandle != nullptr && customNativeHandleSize > 0)
    {
        customNativeHandle_.resize(customNativeHandleSize, UninitializeTag{});
        std::memcpy(customNativeHandle_.get(), customNativeHandle, customNativeHandleSize);
    }
}

std::shared_ptr<GLContext> GLContextManager::AllocContext(
    const GLPixelFormat*    pixelFormat,
    bool                    acceptCompatibleFormat,
    Surface*                surface)
{
    if (pixelFormat != nullptr)
        return FindOrMakeContextWithPixelFormat(*pixelFormat, acceptCompatibleFormat, surface);
    else
        return FindOrMakeAnyContext();
}


/*
 * ======= Private: =======
 */

std::unique_ptr<Surface> GLContextManager::CreatePlaceholderSurface()
{
    #ifdef LLGL_MOBILE_PLATFORM

    /* Create new canvas as placeholder surface */
    return Canvas::Create(CanvasDescriptor{});

    #else

    /* Create new window as placeholder surface */
    WindowDescriptor windowDesc;
    {
        windowDesc.size = { 256, 256 };
    }
    return Window::Create(windowDesc);

    #endif
}

std::shared_ptr<GLContext> GLContextManager::MakeContextWithPixelFormat(const GLPixelFormat& pixelFormat, Surface* surface)
{
    /* Create placeholder surface is none was specified */
    std::unique_ptr<Surface> placeholderSurface;
    if (surface == nullptr)
    {
        placeholderSurface = CreatePlaceholderSurface();
        surface = placeholderSurface.get();
    }

    /* Use shared GL context if there already is one */
    GLContext* sharedContext = (pixelFormats_.empty() ? nullptr : pixelFormats_.front().context.get());

    /* Create new GL context and append to pixel format list */
    GLPixelFormatWithContext formatWithContext;
    {
        formatWithContext.pixelFormat   = pixelFormat;
        formatWithContext.surface       = std::move(placeholderSurface);
        formatWithContext.context       = GLContext::Create(pixelFormat, profile_, *surface, sharedContext, customNativeHandle_);
    }
    pixelFormats_.emplace_back(std::move(formatWithContext));

    std::shared_ptr<GLContext> context = pixelFormats_.back().context;

    /* Load GL extensions for the very first context */
    const bool hasGLCoreProfile = (profile_.contextProfile == OpenGLContextProfile::CoreProfile);
    const bool abortOnFailure   = !profile_.suppressFailedExtensions;
    LoadSupportedOpenGLExtensions(hasGLCoreProfile, abortOnFailure);

    /* Disable all extensions that are unsupported due to incompatible configurations */
    DisableIncompatibleExtensions();

    /* Initialize state manager for new GL context */
    GLStateManager& stateMngr = context->GetStateManager();
    stateMngr.DetermineExtensionsAndLimits();
    InitRenderStates(stateMngr);

    /* Cache new context as the current one */
    GLContext::SetCurrent(context.get());

    /* Invoke callback to register new GLContext */
    if (newContextCallback_)
        newContextCallback_(*context, pixelFormat);

    return context;
}

/*
Returns true if the pixel format 'baseFormat' is considered compatible with 'newFormat', i.e. its bits are greater or equal.
This serves the purpose of reducing the chance of creating more GL contexts as switching between them is very slow
and LLGL does not make any guarantees of how many samples are actually provided when requesting a certain multi-sample format.
*/
static bool IsGLPixelFormatCompatibleWith(const GLPixelFormat& baseFormat, const GLPixelFormat& newFormat)
{
    return
    (
        baseFormat.colorBits   >= newFormat.colorBits    &&
        baseFormat.depthBits   >= newFormat.depthBits    &&
        baseFormat.stencilBits >= newFormat.stencilBits  &&
        baseFormat.samples     >= newFormat.samples
    );
}

std::shared_ptr<GLContext> GLContextManager::FindOrMakeContextWithPixelFormat(const GLPixelFormat& pixelFormat, bool acceptCompatibleFormat, Surface* surface)
{
    /* Try to find pixel format with an exact match first */
    for (const GLPixelFormatWithContext& formatWithContext : pixelFormats_)
    {
        if (formatWithContext.pixelFormat == pixelFormat)
            return formatWithContext.context;
    }

    /* Try to find suitable pixel format that is considered compatible next */
    if (acceptCompatibleFormat)
    {
        for (const GLPixelFormatWithContext& formatWithContext : pixelFormats_)
        {
            if (IsGLPixelFormatCompatibleWith(formatWithContext.pixelFormat, pixelFormat))
                return formatWithContext.context;
        }
    }

    /* Make a new context with the specified pixel format */
    return MakeContextWithPixelFormat(pixelFormat, surface);
}

std::shared_ptr<GLContext> GLContextManager::FindOrMakeAnyContext()
{
    /* Return first context or create a new one with default pixel format */
    if (!pixelFormats_.empty())
        return pixelFormats_.front().context;
    else
        return MakeContextWithPixelFormat(GLPixelFormat{});
}

void GLContextManager::InitRenderStates(GLStateManager& stateMngr)
{
    /* Initialize state manager by clearing its cache; This will query all current GL states managed by the GL backend */
    stateMngr.ClearCache();

    /* D3D11, Vulkan, and Metal always use a fixed restart index for strip topologies */
    #if LLGL_PRIMITIVE_RESTART_FIXED_INDEX
    if (HasExtension(GLExt::ARB_ES3_compatibility))
        stateMngr.Enable(GLState::PrimitiveRestartFixedIndex);
    #endif

    #if LLGL_OPENGL
    /* D3D10+ has this by default */
    if (HasExtension(GLExt::ARB_seamless_cubemap_per_texture))
        stateMngr.Enable(GLState::TextureCubeMapSeamless);
    #endif

    /* D3D10+ uses clock-wise vertex winding per default */
    stateMngr.SetFrontFace(GL_CW);

    /*
    Set pixel storage to byte-alignment (default is word-alignment).
    This is required so that texture formats like RGB (which is not word-aligned) can be used.
    */
    stateMngr.SetPixelStorePack(0, 0, 1);
    stateMngr.SetPixelStoreUnpack(0, 0, 1);
}


} // /namespace LLGL



// ================================================================================
