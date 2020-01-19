/*
 * MTCommandBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_COMMAND_BUFFER_H
#define LLGL_MT_COMMAND_BUFFER_H


#import <MetalKit/MetalKit.h>

#include <LLGL/CommandBuffer.h>
#include <LLGL/StaticLimits.h>
#include "Buffer/MTStagingBufferPool.h"
#include "Buffer/MTTessFactorBuffer.h"
#include "MTEncoderScheduler.h"
#include <vector>


namespace LLGL
{


class MTBuffer;
class MTTexture;
class MTSampler;
class MTRenderTarget;

class MTCommandBuffer final : public CommandBuffer
{

    public:

        /* ----- Common ----- */

        MTCommandBuffer(id<MTLDevice> device, id<MTLCommandQueue> cmdQueue);
        ~MTCommandBuffer();

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

        // Returns the native MTLCommandBuffer object.
        inline id<MTLCommandBuffer> GetNative() const
        {
            return cmdBuffer_;
        }

    private:

        void SetIndexType(bool indexType16Bits);
        void QueueDrawable(id<MTLDrawable> drawable);
        void PresentDrawables();

        void SetBuffer(MTBuffer& bufferMT, std::uint32_t slot, long stageFlags);
        void SetTexture(MTTexture& textureMT, std::uint32_t slot, long stageFlags);
        void SetSampler(MTSampler& samplerMT, std::uint32_t slot, long stageFlags);

        void FillBufferByte1(MTBuffer& bufferMT, const NSRange& range, std::uint8_t value);
        void FillBufferByte4(MTBuffer& bufferMT, const NSRange& range, std::uint32_t value);
        void FillBufferByte4Emulated(MTBuffer& bufferMT, const NSRange& range, std::uint32_t value);
        void FillBufferByte4Accelerated(MTBuffer& bufferMT, const NSRange& range, std::uint32_t value);

        void DispatchTessellatorStage(NSUInteger numPatchesAndInstances);
        id<MTLRenderCommandEncoder> GetRenderEncoderForPatches(NSUInteger numPatches);

        // Dispatches the specified amount of local threads in as large threadgroups as possible.
        void DispatchThreads1D(
            id<MTLComputeCommandEncoder>    computeEncoder,
            id<MTLComputePipelineState>     computePSO,
            NSUInteger                      numThreads
        );

    private:

        struct MTClearValue
        {
            MTLClearColor   color   = MTLClearColorMake(0.0, 0.0, 0.0, 0.0);
            double          depth   = 1.0;
            std::uint32_t   stencil = 0;
        };

    private:

        id<MTLDevice>                   device_                 = nil;
        id<MTLCommandQueue>             cmdQueue_               = nil;
        id<MTLCommandBuffer>            cmdBuffer_              = nil;
        dispatch_semaphore_t            cmdBufferSemaphore_     = nil;

        MTEncoderScheduler              encoderScheduler_;
        std::vector<id<MTLDrawable>>    drawables_;

        MTLPrimitiveType                primitiveType_          = MTLPrimitiveTypeTriangle;
        id<MTLBuffer>                   indexBuffer_            = nil;
        NSUInteger                      indexBufferOffset_      = 0;
        MTLIndexType                    indexType_              = MTLIndexTypeUInt32;
        NSUInteger                      indexTypeSize_          = 4;
        NSUInteger                      numPatchControlPoints_  = 0;
        const MTLSize*                  numThreadsPerGroup_     = nullptr;

        MTClearValue                    clearValue_;

        MTStagingBufferPool             stagingBufferPool_;

        // Tessellator stage objects
        MTTessFactorBuffer              tessFactorBuffer_;
        NSUInteger                      tessFactorBufferSlot_   = 30;
        NSUInteger                      tessFactorSize_         = 0;
        id<MTLComputePipelineState>     tessPipelineState_      = nil;

};


} // /namespace LLGL


#endif



// ================================================================================
