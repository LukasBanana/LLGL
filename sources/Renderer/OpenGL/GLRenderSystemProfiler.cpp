/*
 * GLRenderSystemProfiler.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderSystemProfiler.h"
#include "GLRenderContextProfiler.h"
#include "../../Core/Helper.h"


namespace LLGL
{


GLRenderSystemProfiler::GLRenderSystemProfiler(RenderingProfiler& profiler) :
    profiler_( profiler )
{
}

RenderContext* GLRenderSystemProfiler::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window)
{
    /* Create new render context */
    return AddRenderContext(MakeUnique<GLRenderContextProfiler>(desc, window, nullptr, profiler_), desc, window);
}

/* ----- Hardware buffers ------ */

void GLRenderSystemProfiler::WriteVertexBuffer(
    VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const VertexFormat& vertexFormat)
{
    GLRenderSystem::WriteVertexBuffer(vertexBuffer, data, dataSize, usage, vertexFormat);
    profiler_.writeVertexBuffer.Inc();
}

void GLRenderSystemProfiler::WriteIndexBuffer(
    IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const IndexFormat& indexFormat)
{
    GLRenderSystem::WriteIndexBuffer(indexBuffer, data, dataSize, usage, indexFormat);
    profiler_.writeIndexBuffer.Inc();
}

void GLRenderSystemProfiler::WriteConstantBuffer(
    ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, const BufferUsage usage)
{
    GLRenderSystem::WriteConstantBuffer(constantBuffer, data, dataSize, usage);
    profiler_.writeConstantBuffer.Inc();
}

void GLRenderSystemProfiler::WriteVertexBufferSub(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    GLRenderSystem::WriteVertexBufferSub(vertexBuffer, data, dataSize, offset);
    profiler_.writeVertexBufferSub.Inc();
}

void GLRenderSystemProfiler::WriteIndexBufferSub(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    GLRenderSystem::WriteIndexBufferSub(indexBuffer, data, dataSize, offset);
    profiler_.writeIndexBufferSub.Inc();
}

void GLRenderSystemProfiler::WriteConstantBufferSub(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    GLRenderSystem::WriteConstantBufferSub(constantBuffer, data, dataSize, offset);
    profiler_.writeConstantBufferSub.Inc();
}


} // /namespace LLGL



// ================================================================================
