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

    /* Load all OpenGL extensions for the first time */
    if (renderContexts_.empty())
        LoadGLExtensions(desc.profileOpenGL);

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

ConstantBuffer* GLRenderSystem::CreateConstantBuffer()
{
    return TakeOwnership(constantBuffers_, MakeUnique<GLConstantBuffer>());
}

void GLRenderSystem::WriteVertexBuffer(
    VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const VertexFormat& vertexFormat)
{
    /* Bind vertex buffer */
    auto& vertexBufferGL = LLGL_CAST(GLVertexBuffer&, vertexBuffer);
    GLStateManager::active->BindBuffer(vertexBufferGL);

    /* Update buffer data and update new vertex format */
    vertexBufferGL.hwBuffer.BufferData(data, dataSize, GLTypeConversion::Map(usage));
    vertexBufferGL.UpdateVertexFormat(vertexFormat);
}

void GLRenderSystem::WriteIndexBuffer(
    IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const IndexFormat& indexFormat)
{
    /* Bind index buffer */
    auto& indexBufferGL = LLGL_CAST(GLIndexBuffer&, indexBuffer);
    GLStateManager::active->BindBuffer(indexBufferGL);

    /* Update buffer data and update new index format */
    indexBufferGL.hwBuffer.BufferData(data, dataSize, GLTypeConversion::Map(usage));
    indexBufferGL.UpdateIndexFormat(indexFormat);
}

void GLRenderSystem::WriteConstantBuffer(
    ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, const BufferUsage usage)
{
    /* Bind constant buffer */
    auto& constantBufferGL = LLGL_CAST(GLConstantBuffer&, constantBuffer);
    GLStateManager::active->BindBuffer(constantBufferGL);

    /* Update buffer data */
    constantBufferGL.hwBuffer.BufferData(data, dataSize, GLTypeConversion::Map(usage));
}

void GLRenderSystem::WriteVertexBufferSub(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    /* Bind and update vertex buffer */
    auto& vertexBufferGL = LLGL_CAST(GLVertexBuffer&, vertexBuffer);
    GLStateManager::active->BindBuffer(vertexBufferGL);
    vertexBufferGL.hwBuffer.BufferSubData(data, dataSize, static_cast<GLintptr>(offset));
}

void GLRenderSystem::WriteIndexBufferSub(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    /* Bind index buffer */
    auto& indexBufferGL = LLGL_CAST(GLIndexBuffer&, indexBuffer);
    GLStateManager::active->BindBuffer(indexBufferGL);
    indexBufferGL.hwBuffer.BufferSubData(data, dataSize, static_cast<GLintptr>(offset));
}

void GLRenderSystem::WriteConstantBufferSub(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    /* Bind constant buffer */
    auto& constantBufferGL = LLGL_CAST(GLConstantBuffer&, constantBuffer);
    GLStateManager::active->BindBuffer(constantBufferGL);
    constantBufferGL.hwBuffer.BufferSubData(data, dataSize, static_cast<GLintptr>(offset));
}

/* ----- Shader ----- */

VertexShader* GLRenderSystem::CreateVertexShader()
{
    return TakeOwnership(vertexShaders_, MakeUnique<GLVertexShader>());
}

FragmentShader* GLRenderSystem::CreateFragmentShader()
{
    return TakeOwnership(fragmentShaders_, MakeUnique<GLFragmentShader>());
}

GeometryShader* GLRenderSystem::CreateGeometryShader()
{
    return TakeOwnership(geometryShaders_, MakeUnique<GLGeometryShader>());
}

TessControlShader* GLRenderSystem::CreateTessControlShader()
{
    return TakeOwnership(tessControlShaders_, MakeUnique<GLTessControlShader>());
}

TessEvaluationShader* GLRenderSystem::CreateTessEvaluationShader()
{
    return TakeOwnership(tessEvaluationShaders_, MakeUnique<GLTessEvaluationShader>());
}

ComputeShader* GLRenderSystem::CreateComputeShader()
{
    return TakeOwnership(computeShaders_, MakeUnique<GLComputeShader>());
}

ShaderProgram* GLRenderSystem::CreateShaderProgram()
{
    return TakeOwnership(shaderPrograms_, MakeUnique<GLShaderProgram>());
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

void GLRenderSystem::LoadGLExtensions(const ProfileOpenGLDescriptor& profileDesc)
{
    /* Load OpenGL extensions if not already done */
    if (extensionMap_.empty())
    {
        auto coreProfile = (profileDesc.extProfile && profileDesc.coreProfile);

        /* Query extensions and load all of them */
        extensionMap_ = QueryExtensions(coreProfile);
        LoadAllExtensions(extensionMap_);
    }
}


} // /namespace LLGL



// ================================================================================
