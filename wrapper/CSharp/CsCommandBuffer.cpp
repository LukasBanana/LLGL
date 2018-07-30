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


static const int g_maxNumViewports      = 32;
static const int g_maxNumAttachments    = 16;

CommandBuffer::CommandBuffer(::LLGL::CommandBuffer* native) :
    native_ { native }
{
}

::LLGL::CommandBuffer* CommandBuffer::Native::get()
{
    return native_;
}

/* ----- Encoding ----- */

void CommandBuffer::Begin()
{
    native_->Begin();
}

void CommandBuffer::End()
{
    native_->End();
}

void CommandBuffer::UpdateBuffer(Buffer^ dstBuffer, System::UInt64 dstOffset, System::IntPtr data, System::UInt16 dataSize)
{
    native_->UpdateBuffer(*(dstBuffer->NativeSub), dstOffset, data.ToPointer(), dataSize);
}

void CommandBuffer::CopyBuffer(Buffer^ dstBuffer, System::UInt64 dstOffset, Buffer^ srcBuffer, System::UInt64 srcOffset, System::UInt64 size)
{
    native_->CopyBuffer(*(dstBuffer->NativeSub), dstOffset, *(srcBuffer->NativeSub), srcOffset, size);
}

/* ----- Viewport and Scissor ----- */

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
    ::LLGL::Viewport nativeViewport;
    Convert(nativeViewport, viewport);
    native_->SetViewport(nativeViewport);
}

void CommandBuffer::SetViewports(array<Viewport^>^ viewports)
{
    ::LLGL::Viewport nativeViewports[g_maxNumViewports];

    auto numViewports = static_cast<std::uint32_t>(std::min(viewports->Length, g_maxNumViewports));
    for (std::uint32_t i = 0; i < numViewports; ++i)
        Convert(nativeViewports[i], viewports[i]);

    native_->SetViewports(numViewports, nativeViewports);
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
    ::LLGL::Scissor nativeScissor;
    Convert(nativeScissor, scissor);
    native_->SetScissor(nativeScissor);
}

void CommandBuffer::SetScissors(array<Scissor^>^ scissors)
{
    ::LLGL::Scissor nativeScissors[g_maxNumViewports];

    auto numScissors = static_cast<std::uint32_t>(std::min(scissors->Length, g_maxNumViewports));
    for (std::uint32_t i = 0; i < numScissors; ++i)
        Convert(nativeScissors[i], scissors[i]);

    native_->SetScissors(numScissors, nativeScissors);
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

static void Convert(::LLGL::ColorRGBAf& dst, ColorRGBA^ src)
{
    dst.r = src->R;
    dst.g = src->G;
    dst.b = src->B;
    dst.a = src->A;
}

static void Convert(::LLGL::ClearValue& dst, ClearValue^ src)
{
    Convert(dst.color, src->Color);
    dst.depth   = src->Depth;
    dst.stencil = src->Stencil;
}

static void Convert(::LLGL::AttachmentClear& dst, AttachmentClear^ src)
{
    dst.flags           = static_cast<long>(src->Flags);
    dst.colorAttachment = src->ColorAttachment;
    Convert(dst.clearValue, src->ClearValue);
}

void CommandBuffer::ClearAttachments(array<AttachmentClear^>^ attachments)
{
    ::LLGL::AttachmentClear nativeAttachments[g_maxNumAttachments];

    auto numAttachments = static_cast<std::uint32_t>(std::min(attachments->Length, g_maxNumAttachments));
    for (std::uint32_t i = 0; i < numAttachments; ++i)
        Convert(nativeAttachments[i], attachments[i]);

    native_->ClearAttachments(numAttachments, nativeAttachments);
}

/* ----- Input Assembly ------ */

static ::LLGL::Buffer* GetNative(Buffer^ buffer)
{
    return static_cast<::LLGL::Buffer*>(buffer->Native);
}

void CommandBuffer::SetVertexBuffer(Buffer^ buffer)
{
    native_->SetVertexBuffer(*GetNative(buffer));
}

void CommandBuffer::SetVertexBufferArray(BufferArray^ bufferArray)
{
    native_->SetVertexBufferArray(*bufferArray->Native);
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
    native_->SetGraphicsResourceHeap(*resourceHeap->Native);
}

void CommandBuffer::SetGraphicsResourceHeap(ResourceHeap^ resourceHeap, unsigned int firstSet)
{
    native_->SetGraphicsResourceHeap(*resourceHeap->Native, firstSet);
}

void CommandBuffer::SetComputeResourceHeap(ResourceHeap^ resourceHeap)
{
    native_->SetComputeResourceHeap(*resourceHeap->Native);
}

void CommandBuffer::SetComputeResourceHeap(ResourceHeap^ resourceHeap, unsigned int firstSet)
{
    native_->SetComputeResourceHeap(*resourceHeap->Native, firstSet);
}

/* ----- Render Passes ----- */

void CommandBuffer::BeginRenderPass(RenderTarget^ renderTarget)
{
    native_->BeginRenderPass(*renderTarget->Native);
}

void CommandBuffer::BeginRenderPass(RenderTarget^ renderTarget, RenderPass^ renderPass)
{
    native_->BeginRenderPass(*renderTarget->Native, renderPass->Native);
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
    native_->SetGraphicsPipeline(*graphicsPipeline->Native);
}

void CommandBuffer::SetComputePipeline(ComputePipeline^ computePipeline)
{
    native_->SetComputePipeline(*computePipeline->Native);
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
