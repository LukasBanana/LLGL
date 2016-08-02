/*
 * GLRenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderSystem.h"


namespace LLGL
{


GLRenderSystem::GLRenderSystem()
{
}

GLRenderSystem::~GLRenderSystem()
{
}

RenderContext* GLRenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, Window& window)
{
    /* Create new render context, take ownership, and return the raw pointer */
    auto renderContext = std::unique_ptr<GLRenderContext>(
        new GLRenderContext(desc, window, nullptr)
    );

    renderContexts_.emplace_back(std::move(renderContext));
    
    return renderContexts_.back().get();
}


} // /namespace LLGL



// ================================================================================
