/*
 * GLRenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderSystem.h"
#include "../CheckedCast.h"
#include "../../Core/Helper.h"
#include <LLGL/Desktop.h>
#include "GLStateManager.h"
#include "GLTypeConversion.h"


namespace LLGL
{


GLRenderSystem::GLRenderSystem()
{
}

GLRenderSystem::~GLRenderSystem()
{
    Desktop::ResetVideoMode();
}

RenderContext* GLRenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window)
{
    /* Create new render context */
    auto renderContext = std::unique_ptr<GLRenderContext>(new GLRenderContext(desc, window, nullptr));

    /*
    If render context created it's own window then show it after creation,
    since anti-aliasing may force the window to be recreated several times
    */
    if (!window)
        renderContext->GetWindow().Show();

    /* Switch to fullscreen mode (if enabled) */
    if (desc.videoMode.fullscreen)
        Desktop::ChangeVideoMode(desc.videoMode);

    /* Take ownership and return raw pointer */
    return TakeOwnership(renderContexts_, std::move(renderContext));
}

/* ----- Hardware buffers ------ */

VertexBuffer* GLRenderSystem::CreateVertexBuffer()
{
    return TakeOwnership(vertexBuffers_, MakeUnique<GLVertexBuffer>());
}

IndexBuffer* GLRenderSystem::CreateIndexBuffer()
{
    return TakeOwnership(indexBuffers_, MakeUnique<GLIndexBuffer>());
}

void GLRenderSystem::WriteVertexBuffer(
    VertexBuffer& vertexBuffer,
    const void* data,
    std::size_t dataSize,
    const BufferUsage usage,
    const VertexFormat& vertexFormat)
{
    /* Bind vertex buffer */
    auto& vertexBufferGL = LLGL_CAST(GLVertexBuffer&, vertexBuffer);
    GLStateManager::active->BindBuffer(vertexBufferGL);

    /* Update buffer data */
    vertexBufferGL.hwBuffer.BufferData(data, dataSize, GLTypeConversion::Map(usage));

    /* Setup new vertex format */
    vertexBufferGL.UpdateVertexFormat(vertexFormat);
}

void GLRenderSystem::WriteIndexBuffer(
    IndexBuffer& indexBuffer,
    const void* data,
    std::size_t dataSize,
    const BufferUsage usage,
    const IndexFormat& indexFormat)
{
    /* Bind vertex buffer */
    auto& indexBufferGL = LLGL_CAST(GLIndexBuffer&, indexBuffer);
    GLStateManager::active->BindBuffer(indexBufferGL);

    /* Update buffer data */
    indexBufferGL.hwBuffer.BufferData(data, dataSize, GLTypeConversion::Map(usage));

    /* Setup new index format */
    indexBufferGL.UpdateIndexFormat(indexFormat);
}


/*
 * ======= Private: =======
 */

bool GLRenderSystem::OnMakeCurrent(RenderContext* renderContext)
{
    if (renderContext)
    {
        auto renderContextGL = LLGL_CAST(GLRenderContext*, renderContext);
        return GLRenderContext::GLMakeCurrent(renderContextGL);
    }
    else
        return GLRenderContext::GLMakeCurrent(nullptr);
}


} // /namespace LLGL



// ================================================================================
