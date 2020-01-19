/*
 * D3D12CommandBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_COMMAND_BUFFER_H
#define LLGL_D3D12_COMMAND_BUFFER_H


#include <LLGL/CommandBuffer.h>
#include <cstddef>
#include "D3D12CommandContext.h"
#include "../Buffer/D3D12StagingBufferPool.h"
#include "../../DXCommon/ComPtr.h"
#include "../../DXCommon/DXCore.h"

#include <d3d12.h>
#include <dxgi1_4.h>


namespace LLGL
{


class D3D12RenderSystem;
class D3D12RenderContext;
class D3D12RenderTarget;
class D3D12RenderPass;
class D3D12SignatureFactory;
struct D3D12Resource;

class D3D12CommandBuffer final : public CommandBuffer
{

    public:

        /* ----- Common ----- */

        D3D12CommandBuffer(D3D12RenderSystem& renderSystem, const CommandBufferDescriptor& desc);

        void SetName(const char* name) override;

        /* ----- Encoding ----- */

        void Begin() override;
        void End() override;

        void Execute(CommandBuffer& deferredCommandBuffer) override;

        /* ----- Blitting ----- */

        void UpdateBuffer(
            Buffer&         dstBuffer,
            std::uint64_t   dstOffset,
            const void*     data,
            std::uint16_t   dataSize
        ) override;

        void CopyBuffer(
            Buffer&         dstBuffer,
            std::uint64_t   dstOffset,
            Buffer&         srcBuffer,
            std::uint64_t   srcOffset,
            std::uint64_t   size
        ) override;

        void CopyBufferFromTexture(
            Buffer&                 dstBuffer,
            std::uint64_t           dstOffset,
            Texture&                srcTexture,
            const TextureRegion&    srcRegion,
            std::uint32_t           rowStride   = 0,
            std::uint32_t           layerStride = 0
        ) override;

        void FillBuffer(
            Buffer&         dstBuffer,
            std::uint64_t   dstOffset,
            std::uint32_t   value,
            std::uint64_t   fillSize    = Constants::wholeSize
        ) override;

        void CopyTexture(
            Texture&                dstTexture,
            const TextureLocation&  dstLocation,
            Texture&                srcTexture,
            const TextureLocation&  srcLocation,
            const Extent3D&         extent
        ) override;

        void CopyTextureFromBuffer(
            Texture&                dstTexture,
            const TextureRegion&    dstRegion,
            Buffer&                 srcBuffer,
            std::uint64_t           srcOffset,
            std::uint32_t           rowStride   = 0,
            std::uint32_t           layerStride = 0
        ) override;

        void GenerateMips(Texture& texture) override;
        void GenerateMips(Texture& texture, const TextureSubresource& subresource) override;

        /* ----- Viewport and Scissor ----- */

        void SetViewport(const Viewport& viewport) override;
        void SetViewports(std::uint32_t numViewports, const Viewport* viewports) override;

        void SetScissor(const Scissor& scissor) override;
        void SetScissors(std::uint32_t numScissors, const Scissor* scissors) override;

        /* ----- Clear ----- */

        void SetClearColor(const ColorRGBAf& color) override;
        void SetClearDepth(float depth) override;
        void SetClearStencil(std::uint32_t stencil) override;

        void Clear(long flags) override;
        void ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments) override;

        /* ----- Input Assembly ------ */

        void SetVertexBuffer(Buffer& buffer) override;
        void SetVertexBufferArray(BufferArray& bufferArray) override;

        void SetIndexBuffer(Buffer& buffer) override;
        void SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset = 0) override;

        /* ----- Resources ----- */

        void SetResourceHeap(
            ResourceHeap&           resourceHeap,
            std::uint32_t           firstSet        = 0,
            const PipelineBindPoint bindPoint       = PipelineBindPoint::Undefined
        ) override;

        void SetResource(Resource& resource, std::uint32_t slot, long bindFlags, long stageFlags = StageFlags::AllStages) override;

        void ResetResourceSlots(
            const ResourceType  resourceType,
            std::uint32_t       firstSlot,
            std::uint32_t       numSlots,
            long                bindFlags,
            long                stageFlags      = StageFlags::AllStages
        ) override;

        /* ----- Render Passes ----- */

        void BeginRenderPass(
            RenderTarget&       renderTarget,
            const RenderPass*   renderPass      = nullptr,
            std::uint32_t       numClearValues  = 0,
            const ClearValue*   clearValues     = nullptr
        ) override;

        void EndRenderPass() override;

        /* ----- Pipeline States ----- */

        void SetPipelineState(PipelineState& pipelineState) override;
        void SetBlendFactor(const ColorRGBAf& color) override;
        void SetStencilReference(std::uint32_t reference, const StencilFace stencilFace = StencilFace::FrontAndBack) override;

        void SetUniform(
            UniformLocation location,
            const void*     data,
            std::uint32_t   dataSize
        ) override;

        void SetUniforms(
            UniformLocation location,
            std::uint32_t   count,
            const void*     data,
            std::uint32_t   dataSize
        ) override;

        /* ----- Queries ----- */

        void BeginQuery(QueryHeap& queryHeap, std::uint32_t query = 0) override;
        void EndQuery(QueryHeap& queryHeap, std::uint32_t query = 0) override;

        void BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query = 0, const RenderConditionMode mode = RenderConditionMode::Wait) override;
        void EndRenderCondition() override;

        /* ----- Stream Output ------ */

        void BeginStreamOutput(std::uint32_t numBuffers, Buffer* const * buffers) override;
        void EndStreamOutput() override;

        /* ----- Drawing ----- */

        void Draw(std::uint32_t numVertices, std::uint32_t firstVertex) override;

        void DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex) override;
        void DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset) override;

        void DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances) override;
        void DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance) override;

        void DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex) override;
        void DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset) override;
        void DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance) override;

        void DrawIndirect(Buffer& buffer, std::uint64_t offset) override;
        void DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride) override;

        void DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset) override;
        void DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride) override;

        /* ----- Compute ----- */

        void Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ) override;
        void DispatchIndirect(Buffer& buffer, std::uint64_t offset) override;

        /* ----- Debugging ----- */

        void PushDebugGroup(const char* name) override;
        void PopDebugGroup() override;

        /* ----- Extensions ----- */

        void SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize) override;

    public:

        /* ----- Extended functions ----- */

        // Executes this command buffer.
        void Execute();

        // Returns the native ID3D12GraphicsCommandList object.
        inline ID3D12GraphicsCommandList* GetNative() const
        {
            return commandList_;
        }

    private:

        void CreateCommandContext(D3D12RenderSystem& renderSystem, const CommandBufferDescriptor& desc);

        void SetScissorRectsToDefault(UINT numScissorRects);

        void BindRenderTarget(D3D12RenderTarget& renderTargetD3D);
        void BindRenderContext(D3D12RenderContext& renderContextD3D);

        void ClearAttachmentsWithRenderPass(
            const D3D12RenderPass&  renderPassD3D,
            std::uint32_t           numClearValues,
            const ClearValue*       clearValues,
            UINT                    numRects        = 0,
            const D3D12_RECT*       rects           = nullptr
        );

        void ClearRenderTargetViews(
            const std::uint8_t* colorBuffers,
            std::uint32_t       numClearValues,
            const ClearValue*   clearValues,
            std::uint32_t&      idx,
            UINT                numRects,
            const D3D12_RECT*   rects
        );

        void ClearDepthStencilView(
            D3D12_CLEAR_FLAGS   clearFlags,
            std::uint32_t       numClearValues,
            const ClearValue*   clearValues,
            std::uint32_t       idx,
            UINT                numRects,
            const D3D12_RECT*   rects
        );

    private:

        D3D12CommandContext             commandContext_;
        ID3D12GraphicsCommandList*      commandList_            = nullptr;
        const D3D12SignatureFactory*    cmdSignatureFactory_    = nullptr;

        D3D12StagingBufferPool          stagingBufferPool_;

        D3D12_CPU_DESCRIPTOR_HANDLE     rtvDescHandle_          = {};
        UINT                            rtvDescSize_            = 0;
        D3D12_CPU_DESCRIPTOR_HANDLE     dsvDescHandle_          = {};
        UINT                            dsvDescSize_            = 0;

        ClearValue                      clearValue_;

        bool                            scissorEnabled_         = false;
        UINT                            numBoundScissorRects_   = 0;
        UINT                            numColorBuffers_        = 0;

        RenderTarget*                   boundRenderTarget_      = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
