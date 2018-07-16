/*
 * CsCommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsCommandBuffer.h"
#include <algorithm>


namespace LHermanns
{

namespace LLGL
{


CommandBuffer::CommandBuffer(::LLGL::CommandBuffer* native) :
    native_ { native }
{
}

::LLGL::CommandBuffer* CommandBuffer::Native::get()
{
    return native_;
}

/* ----- Viewport and Scissor ----- */

static const int g_maxNumViewports = 32;

static void Convert(::LLGL::Viewport& dst, Viewport^ src)
{
    dst.x           = src->X;
    dst.y           = src->Y;
    dst.width       = src->Width;
    dst.height      = src->Height;
    dst.minDepth    = src->MinDepth;
    dst.maxDepth    = src->MaxDepth;
}

void CommandBuffer::SetViewport(Viewport^ viewport)
{
    ::LLGL::Viewport viewportNative;
    Convert(viewportNative, viewport);
    native_->SetViewport(viewportNative);
}

void CommandBuffer::SetViewports(array<Viewport^>^ viewports)
{
    ::LLGL::Viewport viewportsNative[g_maxNumViewports];

    auto numViewports = static_cast<std::uint32_t>(std::min(viewports->Length::get(), g_maxNumViewports));
    for (std::uint32_t i = 0; i < numViewports; ++i)
        Convert(viewportsNative[i], viewports[i]);

    native_->SetViewports(numViewports, viewportsNative);
}

static void Convert(::LLGL::Scissor& dst, Scissor^ src)
{
    dst.x       = src->X;
    dst.y       = src->Y;
    dst.width   = src->Width;
    dst.height  = src->Height;
}

void CommandBuffer::SetScissor(Scissor^ scissor)
{
    ::LLGL::Scissor scissorNative;
    Convert(scissorNative, scissor);
    native_->SetScissor(scissorNative);
}

void CommandBuffer::SetScissors(array<Scissor^>^ scissors)
{
    ::LLGL::Scissor scissorsNative[g_maxNumViewports];

    auto numScissors = static_cast<std::uint32_t>(std::min(scissors->Length::get(), g_maxNumViewports));
    for (std::uint32_t i = 0; i < numScissors; ++i)
        Convert(scissorsNative[i], scissors[i]);

    native_->SetScissors(numScissors, scissorsNative);
}

/* ----- Clear ----- */

void CommandBuffer::SetClearColor(float r, float g, float b, float a)
{
    native_->SetClearColor({ r, g, b, a });
}

void CommandBuffer::SetClearDepth(float depth)
{
    native_->SetClearDepth(depth);
}

void CommandBuffer::SetClearStencil(unsigned int stencil)
{
    native_->SetClearStencil(stencil);
}

void CommandBuffer::Clear(int flags)
{
    native_->Clear(static_cast<long>(flags));
}

#if 0
void CommandBuffer::ClearAttachments(array<AttachmentClear^>^ attachments);
#endif

/* ----- Input Assembly ------ */

static ::LLGL::Buffer* GetNative(Buffer^ buffer)
{
    return static_cast<::LLGL::Buffer*>(buffer->Native::get());
}

void CommandBuffer::SetVertexBuffer(Buffer^ buffer)
{
    native_->SetVertexBuffer(*GetNative(buffer));
}

void CommandBuffer::SetVertexBufferArray(BufferArray^ bufferArray)
{
    native_->SetVertexBufferArray(*bufferArray->Native::get());
}

void CommandBuffer::SetIndexBuffer(Buffer^ buffer)
{
    native_->SetIndexBuffer(*GetNative(buffer));
}

#if 0
/* ----- Stream Output Buffers ------ */

void CommandBuffer::SetStreamOutputBuffer(Buffer^ buffer)
{
}

void CommandBuffer::SetStreamOutputBufferArray(BufferArray^ bufferArray)
{
}

void CommandBuffer::BeginStreamOutput(PrimitiveType primitiveType)
{
}

void CommandBuffer::EndStreamOutput()
{
}
#endif

/* ----- Resource Heaps ----- */

void CommandBuffer::SetGraphicsResourceHeap(ResourceHeap^ resourceHeap)
{
    native_->SetGraphicsResourceHeap(*resourceHeap->Native::get());
}

void CommandBuffer::SetGraphicsResourceHeap(ResourceHeap^ resourceHeap, unsigned int firstSet)
{
    native_->SetGraphicsResourceHeap(*resourceHeap->Native::get(), firstSet);
}

void CommandBuffer::SetComputeResourceHeap(ResourceHeap^ resourceHeap)
{
    native_->SetComputeResourceHeap(*resourceHeap->Native::get());
}

void CommandBuffer::SetComputeResourceHeap(ResourceHeap^ resourceHeap, unsigned int firstSet)
{
    native_->SetComputeResourceHeap(*resourceHeap->Native::get(), firstSet);
}

/* ----- Render Passes ----- */

void CommandBuffer::BeginRenderPass(RenderTarget^ renderTarget)
{
    native_->BeginRenderPass(*renderTarget->Native::get());
}

void CommandBuffer::BeginRenderPass(RenderTarget^ renderTarget, RenderPass^ renderPass)
{
    native_->BeginRenderPass(*renderTarget->Native::get(), renderPass->Native::get());
}

#if 0
void CommandBuffer::BeginRenderPass(RenderTarget^ renderTarget, RenderPass^ renderPass, array<ClearValue^>^ clearValues)
{
}
#endif

void CommandBuffer::EndRenderPass()
{
    native_->EndRenderPass();
}

/* ----- Pipeline States ----- */

void CommandBuffer::SetGraphicsPipeline(GraphicsPipeline^ graphicsPipeline)
{
    native_->SetGraphicsPipeline(*graphicsPipeline->Native::get());
}

void CommandBuffer::SetComputePipeline(ComputePipeline^ computePipeline)
{
    native_->SetComputePipeline(*computePipeline->Native::get());
}

/* ----- Drawing ----- */

void CommandBuffer::Draw(unsigned int numVertices, unsigned int firstVertex)
{
    native_->Draw(numVertices, firstVertex);
}

void CommandBuffer::DrawIndexed(unsigned int numIndices, unsigned int firstIndex)
{
    native_->DrawIndexed(numIndices, firstIndex);
}

void CommandBuffer::DrawIndexed(unsigned int numIndices, unsigned int firstIndex, int vertexOffset)
{
    native_->DrawIndexed(numIndices, firstIndex, vertexOffset);
}

void CommandBuffer::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances)
{
    native_->DrawInstanced(numVertices, firstVertex, numInstances);
}

void CommandBuffer::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int firstInstance)
{
    native_->DrawInstanced(numVertices, firstVertex, numInstances, firstInstance);
}

void CommandBuffer::DrawIndexedInstanced(unsigned int numIndices, unsigned int numInstances, unsigned int firstIndex)
{
    native_->DrawIndexedInstanced(numIndices, numInstances, firstIndex);
}

void CommandBuffer::DrawIndexedInstanced(unsigned int numIndices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset)
{
    native_->DrawIndexedInstanced(numIndices, numInstances, firstIndex, vertexOffset);
}

void CommandBuffer::DrawIndexedInstanced(unsigned int numIndices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset, unsigned int firstInstance)
{
    native_->DrawIndexedInstanced(numIndices, numInstances, firstIndex, vertexOffset, firstInstance);
}


/* ----- Compute ----- */

void CommandBuffer::Dispatch(unsigned int groupSizeX, unsigned int groupSizeY, unsigned int groupSizeZ)
{
    native_->Dispatch(groupSizeX, groupSizeY, groupSizeZ);
}


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
