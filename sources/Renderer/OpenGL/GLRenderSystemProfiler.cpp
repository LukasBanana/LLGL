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
    return AddRenderContext(MakeUnique<GLRenderContextProfiler>(*this, desc, window, nullptr, profiler_), desc, window);
}

/* ----- Hardware buffers ------ */


void GLRenderSystemProfiler::WriteVertexBufferSub(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    GLRenderSystem::WriteVertexBufferSub(vertexBuffer, data, dataSize, offset);
    profiler_.updateVertexBuffer.Inc();
}

void GLRenderSystemProfiler::WriteIndexBufferSub(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    GLRenderSystem::WriteIndexBufferSub(indexBuffer, data, dataSize, offset);
    profiler_.updateIndexBuffer.Inc();
}

void GLRenderSystemProfiler::WriteConstantBufferSub(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    GLRenderSystem::WriteConstantBufferSub(constantBuffer, data, dataSize, offset);
    profiler_.updateConstantBuffer.Inc();
}

void GLRenderSystemProfiler::WriteStorageBufferSub(StorageBuffer& storageBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    GLRenderSystem::WriteStorageBufferSub(storageBuffer, data, dataSize, offset);
    profiler_.updateStorageBuffer.Inc();
}


} // /namespace LLGL



// ================================================================================
