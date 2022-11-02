/*
 * GLContextManager.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLContextManager.h"
#include "../RenderState/GLStateManager.h"
#include "../Ext/GLExtensionLoader.h"
#include <LLGL/Window.h>
#include <LLGL/Canvas.h>


namespace LLGL
{


GLContextManager::GLContextManager(const RendererConfigurationOpenGL& profile)
{
    profile_.contextProfile = profile.contextProfile;
    profile_.majorVersion   = profile.majorVersion;
    profile_.minorVersion   = profile.minorVersion;
}

std::shared_ptr<GLContext> GLContextManager::AllocContext(const GLPixelFormat* pixelFormat, Surface* surface)
{
    if (pixelFormat != nullptr)
        return FindOrMakeContextWithPixelFormat(*pixelFormat, surface);
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
        formatWithContext.context       = GLContext::Create(pixelFormat, profile_, *surface, sharedContext);
    }
    pixelFormats_.emplace_back(std::move(formatWithContext));

    auto context = pixelFormats_.back().context;

    /* Load GL extensions for the very first context */
    const bool hasGLCoreProfile = (profile_.contextProfile == OpenGLContextProfile::CoreProfile);
    LoadGLExtensions(hasGLCoreProfile);

    /* Initialize state manager for new GL context */
    auto& stateMngr = context->GetStateManager();
    stateMngr.DetermineExtensionsAndLimits();
    InitRenderStates(stateMngr);

    return context;
}

std::shared_ptr<GLContext> GLContextManager::FindOrMakeContextWithPixelFormat(const GLPixelFormat& pixelFormat, Surface* surface)
{
    /* Try to find suitable pixel format or make new context otherwise */
    for (const auto& formatWithContext : pixelFormats_)
    {
        if (formatWithContext.pixelFormat == pixelFormat)
            return formatWithContext.context;
    }
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
    /* Initialize state manager */
    stateMngr.Reset();

    /* D3D11, Vulkan, and Metal always use a fixed restart index for strip topologies */
    #ifdef LLGL_PRIMITIVE_RESTART_FIXED_INDEX
    stateMngr.Enable(GLState::PRIMITIVE_RESTART_FIXED_INDEX);
    #endif

    #ifdef LLGL_OPENGL
    /* D3D10+ has this per default */
    stateMngr.Enable(GLState::TEXTURE_CUBE_MAP_SEAMLESS);
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


void GLContextManager::LoadGLExtensions(bool hasGLCoreProfile)
{
    /* Load OpenGL extensions if not already done */
    if (!AreExtensionsLoaded())
    {
        /* Query extensions and load all of them */
        auto extensions = QueryExtensions(hasGLCoreProfile);
        LoadAllExtensions(extensions, hasGLCoreProfile);
    }
}


} // /namespace LLGL



// ================================================================================
