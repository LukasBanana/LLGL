/*
 * D3D11CommandBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_COMMAND_BUFFER_H
#define LLGL_D3D11_COMMAND_BUFFER_H


#include <LLGL/CommandBuffer.h>
#include "../DXCommon/ComPtr.h"
#include "../DXCommon/DXCore.h"
#include "Direct3D11.h"
#include <dxgi.h>
#include <vector>
#include <cstddef>


namespace LLGL
{


class D3D11StateManager;
class D3D11RenderTarget;
class D3D11RenderContext;
class D3D11RenderPass;

class D3D11CommandBuffer final : public CommandBuffer
{

    public:

        /* ----- Common ----- */

        D3D11CommandBuffer(
            const ComPtr<ID3D11DeviceContext>&          context,
            const std::shared_ptr<D3D11StateManager>&   stateMngr,
            const CommandBufferDescriptor&              desc
        );

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

        void CopyTexture(
            Texture&                dstTexture,
            const TextureLocation&  dstLocation,
            Texture&                srcTexture,
            const TextureLocation&  srcLocation,
            const Extent3D&         extent
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

        void SetGraphicsResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstSet = 0) override;
        void SetComputeResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstSet = 0) override;

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

        void SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline) override;
        void SetComputePipeline(ComputePipeline& computePipeline) override;

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

        void BeginQuery(QueryHeap& queryHeap, std::uint32_t query) override;
        void EndQuery(QueryHeap& queryHeap, std::uint32_t query) override;

        void BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query, const RenderConditionMode mode) override;
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

    private:

        void SetBuffer(Buffer& buffer, std::uint32_t slot, long bindFlags, long stageFlags);
        void SetTexture(Texture& texture, std::uint32_t slot, long bindFlags, long stageFlags);
        void SetSampler(Sampler& sampler, std::uint32_t slot, long stageFlags);

        void SetConstantBuffersOnStages(UINT startSlot, UINT count, ID3D11Buffer* const* buffers, long stageFlags);
        void SetShaderResourcesOnStages(UINT startSlot, UINT count, ID3D11ShaderResourceView* const* views, long stageFlags);
        void SetSamplersOnStages(UINT startSlot, UINT count, ID3D11SamplerState* const* samplers, long stageFlags);
        void SetUnorderedAccessViewsOnStages(UINT startSlot, UINT count, ID3D11UnorderedAccessView* const* views, const UINT* initialCounts, long stageFlags);

        void ResetBufferResourceSlots(std::uint32_t firstSlot, std::uint32_t numSlots, long bindFlags, long stageFlags);
        void ResetTextureResourceSlots(std::uint32_t firstSlot, std::uint32_t numSlots, long bindFlags, long stageFlags);
        void ResetSamplerResourceSlots(std::uint32_t firstSlot, std::uint32_t numSlots, long bindFlags, long stageFlags);

        void ResetResourceSlotsSRV(std::uint32_t firstSlot, std::uint32_t numSlots, long stageFlags);
        void ResetResourceSlotsUAV(std::uint32_t firstSlot, std::uint32_t numSlots, long stageFlags);

        void ResolveBoundRenderTarget();
        void BindFramebufferView();
        void BindRenderTarget(D3D11RenderTarget& renderTargetD3D);
        void BindRenderContext(D3D11RenderContext& renderContextD3D);

        void ClearAttachmentsWithRenderPass(
            const D3D11RenderPass&  renderPassD3D,
            std::uint32_t           numClearValues,
            const ClearValue*       clearValues
        );

        void ClearColorBuffer(std::uint32_t idx, const ColorRGBAf& color);

        void ClearColorBuffers(
            const std::uint8_t* colorBuffers,
            std::uint32_t       numClearValues,
            const ClearValue*   clearValues,
            std::uint32_t&      idx
        );

    private:

        struct D3D11FramebufferView
        {
            std::vector<ID3D11RenderTargetView*>    rtvList;
            ID3D11DepthStencilView*                 dsv = nullptr;
        };

        ComPtr<ID3D11DeviceContext>         context_;
        bool                                hasDeferredContext_ = false;
        ComPtr<ID3D11CommandList>           commandList_;

        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
        ComPtr<ID3DUserDefinedAnnotation>   annotation_;
        #endif

        std::shared_ptr<D3D11StateManager>  stateMngr_;

        D3D11FramebufferView                framebufferView_;
        D3D11RenderTarget*                  boundRenderTarget_  = nullptr;

        ClearValue                          clearValue_;

};


} // /namespace LLGL


#endif



// ================================================================================
