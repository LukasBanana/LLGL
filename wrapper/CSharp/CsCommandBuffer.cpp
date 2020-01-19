/*
 * CsCommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsCommandBuffer.h"
#include "CsHelper.h"
#include <algorithm>


namespace SharpLLGL
{


/*
 * Common converions
 */

static void Convert(LLGL::ColorRGBAf& dst, ColorRGBA<float>^ src)
{
    dst.r = src->R;
    dst.g = src->G;
    dst.b = src->B;
    dst.a = src->A;
}


/*
 * CommandBuffer class
 */

static const int g_maxNumViewports      = 32;
static const int g_maxNumAttachments    = 16;

CommandBuffer::CommandBuffer(LLGL::CommandBuffer* native) :
    native_ { native }
{
}

LLGL::CommandBuffer* CommandBuffer::Native::get()
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

generic <typename T>
void CommandBuffer::UpdateBuffer(Buffer^ dstBuffer, System::UInt64 dstOffset, array<T>^ data)
{
    pin_ptr<T> dataRef = &data[0];
    native_->UpdateBuffer(*(dstBuffer->NativeSub), dstOffset, dataRef, static_cast<std::uint16_t>(data->Length * sizeof(T)));
}

void CommandBuffer::CopyBuffer(Buffer^ dstBuffer, System::UInt64 dstOffset, Buffer^ srcBuffer, System::UInt64 srcOffset, System::UInt64 size)
{
    native_->CopyBuffer(*(dstBuffer->NativeSub), dstOffset, *(srcBuffer->NativeSub), srcOffset, size);
}

void CommandBuffer::FillBuffer(Buffer^ dstBuffer, System::UInt64 dstOffset, unsigned int value, System::UInt64 fillSize)
{
    native_->FillBuffer(*(dstBuffer->NativeSub), dstOffset, value, fillSize);
}

/* ----- Viewport and Scissor ----- */

static void Convert(LLGL::Viewport& dst, Viewport^ src)
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
    LLGL::Viewport nativeViewport;
    Convert(nativeViewport, viewport);
    native_->SetViewport(nativeViewport);
}

void CommandBuffer::SetViewports(array<Viewport^>^ viewports)
{
    LLGL::Viewport nativeViewports[g_maxNumViewports];

    auto numViewports = static_cast<std::uint32_t>(std::min(viewports->Length, g_maxNumViewports));
    for (std::uint32_t i = 0; i < numViewports; ++i)
        Convert(nativeViewports[i], viewports[i]);

    native_->SetViewports(numViewports, nativeViewports);
}

static void Convert(LLGL::Scissor& dst, Scissor^ src)
{
    dst.x       = src->X;
    dst.y       = src->Y;
    dst.width   = src->Width;
    dst.height  = src->Height;
}

void CommandBuffer::SetScissor(Scissor^ scissor)
{
    LLGL::Scissor nativeScissor;
    Convert(nativeScissor, scissor);
    native_->SetScissor(nativeScissor);
}

void CommandBuffer::SetScissors(array<Scissor^>^ scissors)
{
    LLGL::Scissor nativeScissors[g_maxNumViewports];

    auto numScissors = static_cast<std::uint32_t>(std::min(scissors->Length, g_maxNumViewports));
    for (std::uint32_t i = 0; i < numScissors; ++i)
        Convert(nativeScissors[i], scissors[i]);

    native_->SetScissors(numScissors, nativeScissors);
}

/* ----- Clear ----- */

void CommandBuffer::SetClearColor(ColorRGBA<float>^ color)
{
    native_->SetClearColor({ color->R, color->G, color->B, color->A });
}

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

void CommandBuffer::Clear(ClearFlags flags)
{
    native_->Clear(static_cast<long>(flags));
}

static void Convert(LLGL::ClearValue& dst, ClearValue^ src)
{
    Convert(dst.color, src->Color);
    dst.depth   = src->Depth;
    dst.stencil = src->Stencil;
}

static void Convert(LLGL::AttachmentClear& dst, AttachmentClear^ src)
{
    dst.flags           = static_cast<long>(src->Flags);
    dst.colorAttachment = src->ColorAttachment;
    Convert(dst.clearValue, src->ClearValue);
}

void CommandBuffer::ClearAttachments(array<AttachmentClear^>^ attachments)
{
    LLGL::AttachmentClear nativeAttachments[g_maxNumAttachments];

    auto numAttachments = static_cast<std::uint32_t>(std::min(attachments->Length, g_maxNumAttachments));
    for (std::uint32_t i = 0; i < numAttachments; ++i)
        Convert(nativeAttachments[i], attachments[i]);

    native_->ClearAttachments(numAttachments, nativeAttachments);
}

/* ----- Input Assembly ------ */

static LLGL::Buffer* GetNative(Buffer^ buffer)
{
    return static_cast<LLGL::Buffer*>(buffer->Native);
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

void CommandBuffer::BeginStreamOutput()
{
}

void CommandBuffer::EndStreamOutput()
{
}
#endif

/* ----- Resource Heaps ----- */

void CommandBuffer::SetResourceHeap(ResourceHeap^ resourceHeap)
{
    native_->SetResourceHeap(*resourceHeap->Native);
}

void CommandBuffer::SetResourceHeap(ResourceHeap^ resourceHeap, unsigned int firstSet)
{
    native_->SetResourceHeap(*resourceHeap->Native, firstSet);
}

void CommandBuffer::SetResourceHeap(ResourceHeap^ resourceHeap, unsigned int firstSet, PipelineBindPoint bindPoint)
{
    native_->SetResourceHeap(*resourceHeap->Native, firstSet, static_cast<LLGL::PipelineBindPoint>(bindPoint));
}

/* ----- Render Passes ----- */

void CommandBuffer::BeginRenderPass(RenderTarget^ renderTarget)
{
    native_->BeginRenderPass(*renderTarget->Native);
}

void CommandBuffer::BeginRenderPass(RenderTarget^ renderTarget, RenderPass^ renderPass)
{
    native_->BeginRenderPass(
        *renderTarget->Native,
        (renderPass != nullptr ? renderPass->Native : nullptr)
    );
}

void CommandBuffer::BeginRenderPass(RenderTarget^ renderTarget, RenderPass^ renderPass, array<ClearValue^>^ clearValues)
{
    LLGL::ClearValue nativeClearValues[10];

    std::uint32_t numClearValues = static_cast<std::uint32_t>(std::max(clearValues->Length, 10));
    for (std::uint32_t i = 0; i < numClearValues; ++i)
        Convert(nativeClearValues[i], clearValues[i]);

    native_->BeginRenderPass(
        *renderTarget->Native,
        (renderPass != nullptr ? renderPass->Native : nullptr),
        numClearValues,
        nativeClearValues
    );
}

void CommandBuffer::EndRenderPass()
{
    native_->EndRenderPass();
}

/* ----- Pipeline States ----- */

void CommandBuffer::SetPipelineState(PipelineState^ pipelineState)
{
    native_->SetPipelineState(*pipelineState->Native);
}

void CommandBuffer::SetBlendFactor(ColorRGBA<float>^ color)
{
    LLGL::ColorRGBAf nativeColor{ color->R, color->G, color->B, color->A };
    native_->SetBlendFactor(nativeColor);
}

void CommandBuffer::SetStencilReference(unsigned int reference)
{
    native_->SetStencilReference(reference);
}

/*void CommandBuffer::SetStencilReference(unsigned int reference, const StencilFace stencilFace)
{
    native_->SetStencilReference(reference, static_cast<LLGL::StencilFace>(stencilFace));
}*/

/* ----- Stream Outputs ------ */

void CommandBuffer::BeginStreamOutput(array<Buffer^>^ buffers)
{
    LLGL::Buffer* nativeBuffers[LLGL_MAX_NUM_SO_BUFFERS];

    auto numBuffers = std::min(static_cast<std::uint32_t>(buffers->Length), LLGL_MAX_NUM_SO_BUFFERS);
    for (std::uint32_t i = 0; i < numBuffers; ++i)
        nativeBuffers[i] = buffers[i]->NativeSub;

    native_->BeginStreamOutput(numBuffers, nativeBuffers);
}

void CommandBuffer::EndStreamOutput()
{
    native_->EndStreamOutput();
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

void CommandBuffer::DrawIndirect(Buffer^ buffer, System::UInt64 offset)
{
    native_->DrawIndirect(*(buffer->NativeSub), offset);
}

void CommandBuffer::DrawIndirect(Buffer^ buffer, System::UInt64 offset, unsigned int numCommands, unsigned int stride)
{
    native_->DrawIndirect(*(buffer->NativeSub), offset, numCommands, stride);
}

void CommandBuffer::DrawIndexedIndirect(Buffer^ buffer, System::UInt64 offset)
{
    native_->DrawIndexedIndirect(*(buffer->NativeSub), offset);
}

void CommandBuffer::DrawIndexedIndirect(Buffer^ buffer, System::UInt64 offset, unsigned int numCommands, unsigned int stride)
{
    native_->DrawIndexedIndirect(*(buffer->NativeSub), offset, numCommands, stride);
}

/* ----- Compute ----- */

void CommandBuffer::Dispatch(unsigned int groupSizeX, unsigned int groupSizeY, unsigned int groupSizeZ)
{
    native_->Dispatch(groupSizeX, groupSizeY, groupSizeZ);
}

void CommandBuffer::DispatchIndirect(Buffer^ buffer, System::UInt64 offset)
{
    native_->DispatchIndirect(*(buffer->NativeSub), offset);
}

/* ----- Debugging ----- */

void CommandBuffer::PushDebugGroup(String^ name)
{
    auto nameStr = ToStdString(name);
    native_->PushDebugGroup(nameStr.c_str());
}

void CommandBuffer::PopDebugGroup()
{
    native_->PopDebugGroup();
}

/* ----- Extensions ----- */

generic <typename T>
void CommandBuffer::SetGraphicsAPIDependentState(T stateDesc)
{
    pin_ptr<T> stateDescRef = &stateDesc;
    native_->SetGraphicsAPIDependentState(stateDescRef, sizeof(T));
}


} // /namespace SharpLLGL



// ================================================================================
