/*
 * D3D12CommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12CommandBuffer.h"
#include "D3D12CommandSignaturePool.h"
#include "../D3D12RenderContext.h"
#include "../D3D12RenderSystem.h"
#include "../D3D12Types.h"
#include "../../CheckedCast.h"
#include "../../../Core/Helper.h"

#include "../Buffer/D3D12Buffer.h"
#include "../Buffer/D3D12BufferArray.h"

#include "../Texture/D3D12Texture.h"
#include "../Texture/D3D12RenderTarget.h"

#include "../RenderState/D3D12ResourceHeap.h"
#include "../RenderState/D3D12RenderPass.h"
#include "../RenderState/D3D12QueryHeap.h"

#include "../D3DX12/d3dx12.h"
#include <algorithm>


namespace LLGL
{


D3D12CommandBuffer::D3D12CommandBuffer(D3D12RenderSystem& renderSystem, const CommandBufferDescriptor& desc) :
    commandSignaturePool_ { &(renderSystem.GetCommandSignaturePool()) }
{
    CreateDevices(renderSystem, desc);
}

/* ----- Encoding ----- */

void D3D12CommandBuffer::Begin()
{
    /* Reset command list using the next command allocator */
    NextCommandAllocator();
    auto hr = commandList_->Reset(GetCommandAllocator(), nullptr);
    DXThrowIfFailed(hr, "failed to reset D3D12 graphics command list");
}

void D3D12CommandBuffer::End()
{
    /* Flush remaining resource barriers */
    commandContext_.FlushResourceBarrieres();

    /* Close native command list */
    auto hr = commandList_->Close();
    DXThrowIfFailed(hr, "failed to close D3D12 command list");

    /* Reset intermediate states */
    numBoundScissorRects_ = 0;
}

void D3D12CommandBuffer::UpdateBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint16_t dataSize)
{
    auto& dstBufferD3D = LLGL_CAST(D3D12Buffer&, dstBuffer);
    dstBufferD3D.UpdateDynamicSubresource(commandContext_, data, static_cast<UINT64>(dataSize), dstOffset);
}

void D3D12CommandBuffer::CopyBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, Buffer& srcBuffer, std::uint64_t srcOffset, std::uint64_t size)
{
    auto& dstBufferD3D = LLGL_CAST(D3D12Buffer&, dstBuffer);
    auto& srcBufferD3D = LLGL_CAST(D3D12Buffer&, srcBuffer);
    commandList_->CopyBufferRegion(dstBufferD3D.GetNative(), dstOffset, srcBufferD3D.GetNative(), srcOffset, size);
}

void D3D12CommandBuffer::Execute(CommandBuffer& deferredCommandBuffer)
{
    auto& cmdBufferD3D = LLGL_CAST(D3D12CommandBuffer&, deferredCommandBuffer);
    commandList_->ExecuteBundle(cmdBufferD3D.GetNative());
}

/* ----- Configuration ----- */

void D3D12CommandBuffer::SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize)
{
    // dummy
}

/* ----- Viewport and Scissor ----- */

void D3D12CommandBuffer::SetViewport(const Viewport& viewport)
{
    D3D12CommandBuffer::SetViewports(1, &viewport);
}

void D3D12CommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    numViewports = std::min(numViewports, std::uint32_t(D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE));

    /* Check if D3D12_VIEWPORT and Viewport structures can be safely reinterpret-casted */
    if ( sizeof(D3D12_VIEWPORT)             == sizeof(Viewport)             &&
         offsetof(D3D12_VIEWPORT, TopLeftX) == offsetof(Viewport, x       ) &&
         offsetof(D3D12_VIEWPORT, TopLeftY) == offsetof(Viewport, y       ) &&
         offsetof(D3D12_VIEWPORT, Width   ) == offsetof(Viewport, width   ) &&
         offsetof(D3D12_VIEWPORT, Height  ) == offsetof(Viewport, height  ) &&
         offsetof(D3D12_VIEWPORT, MinDepth) == offsetof(Viewport, minDepth) &&
         offsetof(D3D12_VIEWPORT, MaxDepth) == offsetof(Viewport, maxDepth) )
    {
        /* Now it's safe to reinterpret cast the viewports into D3D viewports */
        commandList_->RSSetViewports(numViewports, reinterpret_cast<const D3D12_VIEWPORT*>(viewports));
    }
    else
    {
        /* Convert viewport into D3D viewport */
        D3D12_VIEWPORT viewportsD3D[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

        for (std::uint32_t i = 0; i < numViewports; ++i)
        {
            const auto& src = viewports[i];
            auto& dest = viewportsD3D[i];

            dest.TopLeftX   = src.x;
            dest.TopLeftY   = src.y;
            dest.Width      = src.width;
            dest.Height     = src.height;
            dest.MinDepth   = src.minDepth;
            dest.MaxDepth   = src.maxDepth;
        }

        commandList_->RSSetViewports(numViewports, viewportsD3D);
    }

    /* If scissor test is disabled, update remaining scissor rectangles to default value */
    if (!scissorEnabled_)
        SetScissorRectsToDefault(numViewports);
}

void D3D12CommandBuffer::SetScissor(const Scissor& scissor)
{
    D3D12CommandBuffer::SetScissors(1, &scissor);
}

void D3D12CommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    numScissors = std::min(numScissors, std::uint32_t(D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE));

    D3D12_RECT scissorsD3D[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

    for (std::uint32_t i = 0; i < numScissors; ++i)
    {
        const auto& src = scissors[i];
        auto& dest = scissorsD3D[i];

        dest.left   = src.x;
        dest.top    = src.y;
        dest.right  = src.x + src.width;
        dest.bottom = src.y + src.height;
    }

    commandList_->RSSetScissorRects(numScissors, scissorsD3D);
}

/* ----- Clear ----- */

void D3D12CommandBuffer::SetClearColor(const ColorRGBAf& color)
{
    clearValue_.color = color;
}

void D3D12CommandBuffer::SetClearDepth(float depth)
{
    clearValue_.depth = depth;
}

void D3D12CommandBuffer::SetClearStencil(std::uint32_t stencil)
{
    clearValue_.stencil = (stencil & 0xff);
}

static D3D12_CLEAR_FLAGS GetClearFlagsDSV(long flags)
{
    UINT clearFlagsDSV = 0;

    if ((flags & ClearFlags::Depth) != 0)
        clearFlagsDSV |= D3D12_CLEAR_FLAG_DEPTH;
    if ((flags & ClearFlags::Stencil) != 0)
        clearFlagsDSV |= D3D12_CLEAR_FLAG_STENCIL;

    return static_cast<D3D12_CLEAR_FLAGS>(clearFlagsDSV);
}

void D3D12CommandBuffer::Clear(long flags)
{
    if (rtvDescHandle_.ptr != 0)
    {
        /* Clear color buffers */
        if ((flags & ClearFlags::Color) != 0)
            commandList_->ClearRenderTargetView(rtvDescHandle_, clearValue_.color.Ptr(), 0, nullptr);
    }

    if (dsvDescHandle_.ptr != 0)
    {
        /* Clear depth-stencil buffer */
        if (auto clearFlagsDSV = GetClearFlagsDSV(flags))
        {
            commandList_->ClearDepthStencilView(
                dsvDescHandle_,
                clearFlagsDSV,
                clearValue_.depth,
                static_cast<UINT8>(clearValue_.stencil),
                0,
                nullptr
            );
        }
    }
}

void D3D12CommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    //TODO...
    //CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescHandle_, targetIndex, rtvDescHandleSize_);
    //commandList_->ClearRenderTargetView(rtvDescHandle_, clearState_.color.Ptr(), 0, nullptr);
}

/* ----- Buffers ------ */

void D3D12CommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    commandList_->IASetVertexBuffers(0, 1, &(bufferD3D.GetVertexBufferView()));
}

void D3D12CommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    auto& bufferArrayD3D = LLGL_CAST(D3D12BufferArray&, bufferArray);
    commandList_->IASetVertexBuffers(
        0,
        static_cast<UINT>(bufferArrayD3D.GetVertexBufferViews().size()),
        bufferArrayD3D.GetVertexBufferViews().data()
    );
}

void D3D12CommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    commandList_->IASetIndexBuffer(&(bufferD3D.GetIndexBufferView()));
}

void D3D12CommandBuffer::SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    auto indexBufferView = bufferD3D.GetIndexBufferView();
    if (indexBufferView.SizeInBytes > offset)
    {
        /* Update buffer location and size by offset, and override format */
        indexBufferView.BufferLocation  += offset;
        indexBufferView.SizeInBytes     -= static_cast<UINT>(offset);
        indexBufferView.Format          = D3D12Types::Map(format);
        commandList_->IASetIndexBuffer(&(bufferD3D.GetIndexBufferView()));
    }
}

/* ----- Stream Output Buffers ------ */

void D3D12CommandBuffer::SetStreamOutputBuffer(Buffer& buffer)
{
    //todo...
}

void D3D12CommandBuffer::SetStreamOutputBufferArray(BufferArray& bufferArray)
{
    //todo...
}

void D3D12CommandBuffer::BeginStreamOutput(const PrimitiveType primitiveType)
{
    // dummy
}

void D3D12CommandBuffer::EndStreamOutput()
{
    // dummy
}

/* ----- Resource Heaps ----- */

void D3D12CommandBuffer::SetGraphicsResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstSet)
{
    /* Get descriptor heaps */
    auto& resourceHeapD3D = LLGL_CAST(D3D12ResourceHeap&, resourceHeap);

    auto descHeaps = resourceHeapD3D.GetDescriptorHeaps();
    auto heapCount = resourceHeapD3D.GetNumDescriptorHeaps();

    if (heapCount > 0)
    {
        /* Bind descriptor heaps */
        commandList_->SetDescriptorHeaps(heapCount, descHeaps);

        /* Bind root descriptor tables to graphics pipeline */
        for (UINT i = 0; i < heapCount; ++i)
            commandList_->SetGraphicsRootDescriptorTable(i, descHeaps[i]->GetGPUDescriptorHandleForHeapStart());
    }
}

void D3D12CommandBuffer::SetComputeResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstSet)
{
    //todo...
}

/* ----- Render Passes ----- */

void D3D12CommandBuffer::BeginRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues)
{
    boundRenderTarget_ = &(renderTarget);

    /* Bind render target/context */
    if (renderTarget.IsRenderContext())
        BindRenderContext(LLGL_CAST(D3D12RenderContext&, renderTarget));
    else
        BindRenderTarget(LLGL_CAST(D3D12RenderTarget&, renderTarget));

    /* Clear attachments */
    if (renderPass)
    {
        auto renderPassD3D = LLGL_CAST(const D3D12RenderPass*, renderPass);
        ClearAttachmentsWithRenderPass(*renderPassD3D, numClearValues, clearValues);
    }
}

void D3D12CommandBuffer::EndRenderPass()
{
    if (boundRenderTarget_)
    {
        if (boundRenderTarget_->IsRenderContext())
        {
            auto renderContextD3D = LLGL_CAST(D3D12RenderContext*, boundRenderTarget_);
            renderContextD3D->ResolveRenderTarget(commandContext_);
        }
        else
        {
            auto renderTargetD3D = LLGL_CAST(D3D12RenderTarget*, boundRenderTarget_);
            renderTargetD3D->ResolveRenderTarget(commandContext_);
        }
        boundRenderTarget_ = nullptr;
    }
}

/* ----- Pipeline States ----- */

void D3D12CommandBuffer::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    /* Set graphics root signature, graphics pipeline state, and primitive topology */
    auto& graphicsPipelineD3D = LLGL_CAST(D3D12GraphicsPipeline&, graphicsPipeline);
    graphicsPipelineD3D.Bind(commandList_.Get());

    /* Scissor rectangle must be updated (if scissor test is disabled) */
    scissorEnabled_ = graphicsPipelineD3D.IsScissorEnabled();
    if (!scissorEnabled_ && commandList_->GetType() == D3D12_COMMAND_LIST_TYPE_DIRECT)
        SetScissorRectsToDefault(graphicsPipelineD3D.NumDefaultScissorRects());
}

void D3D12CommandBuffer::SetComputePipeline(ComputePipeline& computePipeline)
{
    //TODO
}

void D3D12CommandBuffer::SetUniform(
    UniformLocation location,
    const void*     data,
    std::uint32_t   dataSize)
{
    D3D12CommandBuffer::SetUniforms(location, 1, data, dataSize);
}

void D3D12CommandBuffer::SetUniforms(
    UniformLocation location,
    std::uint32_t   count,
    const void*     data,
    std::uint32_t   dataSize)
{
    //TODO
}

/* ----- Queries ----- */

void D3D12CommandBuffer::BeginQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    auto& queryHeapD3D = LLGL_CAST(D3D12QueryHeap&, queryHeap);
    commandList_->BeginQuery(queryHeapD3D.GetNative(), queryHeapD3D.GetNativeType(), query);
}

void D3D12CommandBuffer::EndQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    auto& queryHeapD3D = LLGL_CAST(D3D12QueryHeap&, queryHeap);
    commandList_->EndQuery(queryHeapD3D.GetNative(), queryHeapD3D.GetNativeType(), query);
    queryHeapD3D.ResolveData(commandList_.Get(), query, 1);
}

static D3D12_PREDICATION_OP GetDXPredicateOp(const RenderConditionMode mode)
{
    if (mode >= RenderConditionMode::WaitInverted)
        return D3D12_PREDICATION_OP_NOT_EQUAL_ZERO;
    else
        return D3D12_PREDICATION_OP_EQUAL_ZERO;
}

void D3D12CommandBuffer::BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query, const RenderConditionMode mode)
{
    auto& queryHeapD3D = LLGL_CAST(D3D12QueryHeap&, queryHeap);
    commandList_->SetPredication(
        queryHeapD3D.GetResultResource(),
        queryHeapD3D.GetAlignedBufferOffest(query),
        GetDXPredicateOp(mode)
    );
}

void D3D12CommandBuffer::EndRenderCondition()
{
    commandList_->SetPredication(nullptr, 0, D3D12_PREDICATION_OP_EQUAL_ZERO);
}

/* ----- Drawing ----- */

void D3D12CommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    commandList_->DrawInstanced(numVertices, 1, firstVertex, 0);
}

void D3D12CommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    commandList_->DrawIndexedInstanced(numIndices, 1, firstIndex, 0, 0);
}

void D3D12CommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    commandList_->DrawIndexedInstanced(numIndices, 1, firstIndex, vertexOffset, 0);
}

void D3D12CommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    commandList_->DrawInstanced(numVertices, numInstances, firstVertex, 0);
}

void D3D12CommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    commandList_->DrawInstanced(numVertices, numInstances, firstVertex, firstInstance);
}

void D3D12CommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    commandList_->DrawIndexedInstanced(numIndices, numInstances, firstIndex, 0, 0);
}

void D3D12CommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    commandList_->DrawIndexedInstanced(numIndices, numInstances, firstIndex, vertexOffset, 0);
}

void D3D12CommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    commandList_->DrawIndexedInstanced(numIndices, numInstances, firstIndex, vertexOffset, firstInstance);
}

void D3D12CommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    commandList_->ExecuteIndirect(
        commandSignaturePool_->GetSignatureDrawIndirect(), 1, bufferD3D.GetNative(), offset, nullptr, 0
    );
}

void D3D12CommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    while (numCommands-- > 0)
    {
        commandList_->ExecuteIndirect(
            commandSignaturePool_->GetSignatureDrawIndirect(), 1, bufferD3D.GetNative(), offset, nullptr, 0
        );
        offset += stride;
    }
}

void D3D12CommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    commandList_->ExecuteIndirect(
        commandSignaturePool_->GetSignatureDrawIndexedIndirect(), 1, bufferD3D.GetNative(), offset, nullptr, 0
    );
}

void D3D12CommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    while (numCommands-- > 0)
    {
        commandList_->ExecuteIndirect(
            commandSignaturePool_->GetSignatureDrawIndexedIndirect(), 1, bufferD3D.GetNative(), offset, nullptr, 0
        );
        offset += stride;
    }
}

/* ----- Compute ----- */

void D3D12CommandBuffer::Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ)
{
    commandList_->Dispatch(numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);
}

void D3D12CommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    commandList_->ExecuteIndirect(
        commandSignaturePool_->GetSignatureDispatchIndirect(), 1, bufferD3D.GetNative(), offset, nullptr, 0
    );
}

/* ----- Debugging ----- */

void D3D12CommandBuffer::PushDebugGroup(const char* name)
{
    //TODO
}

void D3D12CommandBuffer::PopDebugGroup()
{
    //TODO
}


/*
 * ======= Private: =======
 */

static D3D12_COMMAND_LIST_TYPE GetD3DCommandListType(const CommandBufferDescriptor& desc)
{
    if ((desc.flags & CommandBufferFlags::DeferredSubmit) != 0)
        return D3D12_COMMAND_LIST_TYPE_BUNDLE;
    else
        return D3D12_COMMAND_LIST_TYPE_DIRECT;
}

void D3D12CommandBuffer::CreateDevices(D3D12RenderSystem& renderSystem, const CommandBufferDescriptor& desc)
{
    auto& device = renderSystem.GetDevice();

    /* Determine number of command allocators */
    numAllocators_ = std::max(1u, std::min(desc.numNativeBuffers, g_maxNumAllocators));

    /* Create command allocators */
    auto listType = GetD3DCommandListType(desc);

    for (std::uint32_t i = 0; i < numAllocators_; ++i)
    {
        auto& cmdAlloc = cmdAllocators_[i];
        cmdAlloc = device.CreateDXCommandAllocator(listType);
        #ifdef LLGL_DEBUG
        std::wstring name = L"LLGL::D3D12CommandBuffer::commandAllocator[" + std::to_wstring(i) + L"]";
        cmdAlloc->SetName(name.c_str());
        #endif
    }

    /* Create graphics command list and close it (they are created in recording mode) */
    commandList_ = device.CreateDXCommandList(listType, GetCommandAllocator());
    commandList_->Close();

    /* Initialize command context */
    commandContext_.SetCommandList(commandList_.Get());
}

void D3D12CommandBuffer::NextCommandAllocator()
{
    /* Get next command allocator */
    currentAllocator_ = ((currentAllocator_ + 1) % numAllocators_);

    /* Reclaim memory allocated by command allocator using <ID3D12CommandAllocator::Reset> */
    auto hr = GetCommandAllocator()->Reset();
    DXThrowIfFailed(hr, "failed to reset D3D12 command allocator");
}

void D3D12CommandBuffer::SetScissorRectsToDefault(UINT numScissorRects)
{
    numScissorRects = std::min(numScissorRects, UINT(D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE));

    if (numScissorRects > numBoundScissorRects_)
    {
        /* Set scissor to render target resolution */
        D3D12_RECT scissorRects[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

        for (UINT i = 0; i < numScissorRects; ++i)
        {
            scissorRects[i].left    = 0;
            scissorRects[i].top     = 0;
            scissorRects[i].right   = std::numeric_limits<LONG>::max();
            scissorRects[i].bottom  = std::numeric_limits<LONG>::max();
        }

        commandList_->RSSetScissorRects(numScissorRects, scissorRects);

        /* Store new number of bound scissor rectangles */
        numBoundScissorRects_ = numScissorRects;
    }
}

void D3D12CommandBuffer::BindRenderTarget(D3D12RenderTarget& renderTargetD3D)
{
    /* Transition resources to state ready for output merger */
    renderTargetD3D.TransitionToOutputMerger(commandContext_);

    /* Set current back buffer as RTV and optional DSV */
    const UINT numRenderTargets = renderTargetD3D.GetNumColorAttachments();

    rtvDescHandle_ = renderTargetD3D.GetCPUDescriptorHandleForRTV();
    dsvDescHandle_ = renderTargetD3D.GetCPUDescriptorHandleForDSV();

    if (dsvDescHandle_.ptr != 0)
        commandList_->OMSetRenderTargets(numRenderTargets, &rtvDescHandle_, TRUE, &dsvDescHandle_);
    else
        commandList_->OMSetRenderTargets(numRenderTargets, &rtvDescHandle_, TRUE, nullptr);
}

void D3D12CommandBuffer::BindRenderContext(D3D12RenderContext& renderContextD3D)
{
    /* Indicate that the back buffer will be used as render target */
    commandContext_.TransitionResource(
        renderContextD3D.GetCurrentColorBuffer(),
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );

    /* Set current back buffer as RTV and optional DSV */
    rtvDescHandle_ = renderContextD3D.GetCPUDescriptorHandleForRTV();
    dsvDescHandle_ = renderContextD3D.GetCPUDescriptorHandleForDSV();

    if (dsvDescHandle_.ptr != 0)
        commandList_->OMSetRenderTargets(1, &rtvDescHandle_, FALSE, &dsvDescHandle_);
    else
        commandList_->OMSetRenderTargets(1, &rtvDescHandle_, FALSE, nullptr);
}

void D3D12CommandBuffer::ClearAttachmentsWithRenderPass(
    const D3D12RenderPass&  renderPassD3D,
    std::uint32_t           numClearValues,
    const ClearValue*       clearValues)
{
    /* Clear color attachments */
    std::uint32_t idx = 0;
    ClearColorBuffers(renderPassD3D.GetClearColorAttachments(), numClearValues, clearValues, idx);

    /* Clear depth-stencil attachment */
    if (dsvDescHandle_.ptr != 0)
    {
        /* Clear depth-stencil buffer */
        if (auto clearFlagsDSV = renderPassD3D.GetClearFlagsDSV())
        {
            /* Get clear values */
            FLOAT depth     = clearValue_.depth;
            UINT8 stencil   = static_cast<UINT8>(clearValue_.stencil);

            if (idx < numClearValues)
            {
                depth   = clearValues[idx].depth;
                stencil = static_cast<UINT8>(clearValues[idx].stencil & 0xff);
            }

            /* Clear depth-stencil view */
            commandList_->ClearDepthStencilView(dsvDescHandle_, clearFlagsDSV, depth, stencil, 0, nullptr);
        }
    }
}

void D3D12CommandBuffer::ClearColorBuffers(
    const std::uint8_t* colorBuffers,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues,
    std::uint32_t&      idx)
{
    //TODO: get correct number for RTVs
    //                       |
    //                       v
    std::uint32_t i = 0, n = 1;

    numClearValues = std::min(numClearValues, n);

    /* Use specified clear values */
    for (; i < numClearValues; ++i)
    {
        /* Check if attachment list has ended */
        if (colorBuffers[i] != 0xFF)
        {
            //TODO: use 'colorBuffers[i]' to select correct <D3D12_CPU_DESCRIPTOR_HANDLE>
            if (rtvDescHandle_.ptr != 0)
            {
                commandList_->ClearRenderTargetView(rtvDescHandle_, clearValues[idx++].color.Ptr(), 0, nullptr);
            }
            else
                ++idx;
        }
        else
            return;
    }

    /* Use default clear values */
    for (; i < n; ++i)
    {
        /* Check if attachment list has ended */
        if (colorBuffers[i] != 0xFF)
        {
            //TODO: use 'colorBuffers[i]' to select correct <D3D12_CPU_DESCRIPTOR_HANDLE>
            if (rtvDescHandle_.ptr != 0)
            {
                commandList_->ClearRenderTargetView(rtvDescHandle_, clearValue_.color.Ptr(), 0, nullptr);
            }
        }
        else
            return;
    }
}


} // /namespace LLGL



// ================================================================================
