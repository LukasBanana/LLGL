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
            ID3D11Device*                               device,
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

    public:

        /* ----- Internal ----- */

        // Returns the native command list for deferred contexts or null if there is none.
        inline ID3D11CommandList* GetDeferredCommandList() const
        {
            return commandList_.Get();
        }

        // Returns true if this is a secondary command buffer that can be executed within a primary command buffer.
        inline bool IsSecondaryCmdBuffer() const
        {
            return isSecondaryCmdBuffer_;
        }

    private:

        void SetBuffer(Buffer& buffer, std::uint32_t slot, long bindFlags, long stageFlags);
        void SetTexture(Texture& texture, std::uint32_t slot, long bindFlags, long stageFlags);
        void SetSampler(Sampler& sampler, std::uint32_t slot, long stageFlags);

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

        void ClearWithIntermediateUAV(ID3D11Buffer* buffer, UINT offset, UINT size, const UINT (&valuesVec4)[4]);

        // Creates a copy of this buffer as ByteAddressBuffer; 'size' must be a multiple of 4.
        void CreateByteAddressBufferR32Typeless(
            ID3D11Device*               device,
            ID3D11DeviceContext*        context,
            ID3D11Buffer**              bufferOutput,
            ID3D11ShaderResourceView**  srvOutput,
            ID3D11UnorderedAccessView** uavOutput,
            UINT                        size,
            D3D11_USAGE                 usage           = D3D11_USAGE_DEFAULT
        );

    private:

        struct D3D11FramebufferView
        {
            std::vector<ID3D11RenderTargetView*>    rtvList;
            ID3D11DepthStencilView*                 dsv = nullptr;
        };

    private:

        // Device object to create on-demand objects like temporary SRVs and UAVs
        ID3D11Device*                       device_                 = nullptr;

        // Primary D3D11 context for most commands
        ComPtr<ID3D11DeviceContext>         context_;

        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
        // Extended D3D11 context to bind constant-buffer ranges (Direct3D 11.1)
        ComPtr<ID3D11DeviceContext1>        context1_;
        #endif

        ComPtr<ID3D11CommandList>           commandList_;

        bool                                hasDeferredContext_     = false;
        bool                                isSecondaryCmdBuffer_   = false;

        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
        ComPtr<ID3DUserDefinedAnnotation>   annotation_;
        #endif

        std::shared_ptr<D3D11StateManager>  stateMngr_;

        D3D11FramebufferView                framebufferView_;
        D3D11RenderTarget*                  boundRenderTarget_      = nullptr;

        ClearValue                          clearValue_;

};


} // /namespace LLGL


#endif



// ================================================================================
