/*
 * GLRenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderSystem.h"
#include "../CheckedCast.h"


namespace LLGL
{


GLRenderSystem::GLRenderSystem()
{
}

GLRenderSystem::~GLRenderSystem()
{
}

RenderContext* GLRenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window)
{
    /* Create new render context, take ownership, and return the raw pointer */
    auto renderContext = std::unique_ptr<GLRenderContext>(
        new GLRenderContext(desc, window, nullptr)
    );

    /*
    If render context created it's own window then show it after creation,
    since anti-aliasing may force the window to be recreated several times
    */
    if (!window)
        renderContext->GetWindow().Show();

    renderContexts_.emplace_back(std::move(renderContext));

    return renderContexts_.back().get();
}


/*
 * ======= Private: =======
 */

void GLRenderSystem::OnMakeCurrent(RenderContext* renderContext)
{
    if (renderContext)
    {
        auto renderContextGL = LLGL_CAST(GLRenderContext*, renderContext);
        GLRenderContext::GLMakeCurrent(renderContextGL);
    }
    else
        GLRenderContext::GLMakeCurrent(nullptr);
}


} // /namespace LLGL



// ================================================================================
