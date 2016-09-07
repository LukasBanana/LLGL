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


void GLRenderSystemProfiler::WriteVertexBuffer(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    GLRenderSystem::WriteVertexBuffer(vertexBuffer, data, dataSize, offset);
    profiler_.writeVertexBuffer.Inc();
}

void GLRenderSystemProfiler::WriteIndexBuffer(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    GLRenderSystem::WriteIndexBuffer(indexBuffer, data, dataSize, offset);
    profiler_.writeIndexBuffer.Inc();
}

void GLRenderSystemProfiler::WriteConstantBuffer(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    GLRenderSystem::WriteConstantBuffer(constantBuffer, data, dataSize, offset);
    profiler_.writeConstantBuffer.Inc();
}

void GLRenderSystemProfiler::WriteStorageBuffer(StorageBuffer& storageBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    GLRenderSystem::WriteStorageBuffer(storageBuffer, data, dataSize, offset);
    profiler_.writeStorageBuffer.Inc();
}


} // /namespace LLGL



// ================================================================================
