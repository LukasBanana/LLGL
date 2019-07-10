/*
 * GLDeferredCommandBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_DEFERRED_COMMAND_BUFFER_H
#define LLGL_GL_DEFERRED_COMMAND_BUFFER_H


#include "GLCommandBuffer.h"
#include "GLCommandOpcode.h"
#include "../RenderState/GLState.h"
#include "../OpenGL.h"
#include <memory>
#include <vector>

#ifdef LLGL_ENABLE_JIT_COMPILER
#   include "../../../JIT/JITProgram.h"
#endif


namespace LLGL
{


class GLRenderTarget;
class GLRenderContext;
class GLStateManager;
class GLRenderPass;

class GLDeferredCommandBuffer final : public GLCommandBuffer
{

    public:

        GLDeferredCommandBuffer(long flags, std::size_t reservedSize = 0);

        bool IsImmediateCmdBuffer() const override;

        /* ----- Encoding ----- */

        void Begin() override;
        void End() override;

        void UpdateBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint16_t dataSize) override;
        void CopyBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, Buffer& srcBuffer, std::uint64_t srcOffset, std::uint64_t size) override;

        void Execute(CommandBuffer& deferredCommandBuffer) override;

        /* ----- Configuration ----- */

        void SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize) override;

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

        /* ----- Stream Output Buffers ------ */

        void SetStreamOutputBuffer(Buffer& buffer) override;
        void SetStreamOutputBufferArray(BufferArray& bufferArray) override;

        void BeginStreamOutput(const PrimitiveType primitiveType) override;
        void EndStreamOutput() override;

        /* ----- Resource Heaps ----- */

        void SetGraphicsResourceHeap(ResourceHeap& resourceHeap, std::uint32_t startSlot) override;
        void SetComputeResourceHeap(ResourceHeap& resourceHeap, std::uint32_t startSlot) override;

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

        void BeginQuery(QueryHeap& queryHeap, std::uint32_t query = 0) override;
        void EndQuery(QueryHeap& queryHeap, std::uint32_t query = 0) override;

        void BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query = 0, const RenderConditionMode mode = RenderConditionMode::Wait) override;
        void EndRenderCondition() override;

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

        /* ----- Direct Resource Access ------ */

        void SetConstantBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags = StageFlags::AllStages) override;
        void SetSampleBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags = StageFlags::AllStages) override;
        void SetRWStorageBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags = StageFlags::AllStages) override;
        void SetTexture(Texture& texture, std::uint32_t layer, long stageFlags = StageFlags::AllStages) override;
        void SetSampler(Sampler& sampler, std::uint32_t layer, long stageFlags = StageFlags::AllStages) override;

        void ResetResourceSlots(
            const ResourceType  resourceType,
            std::uint32_t       firstSlot,
            std::uint32_t       numSlots,
            long                bindFlags,
            long                stageFlags      = StageFlags::AllStages
        ) override;

    public:

        /*  ----- Extended functions ----- */

        // Returns true if this is a primary command buffer.
        bool IsPrimary() const;

        // Returns the internal command buffer as raw byte buffer.
        inline const std::vector<std::uint8_t>& GetRawBuffer() const
        {
            return buffer_;
        }

        // Returns the flags this command buffer was created with (see CommandBufferDescriptor::flags).
        inline long GetFlags() const
        {
            return flags_;
        }

        #ifdef LLGL_ENABLE_JIT_COMPILER

        // Returns the just-in-time compiled command buffer that can be executed natively, or null if not available.
        inline const std::unique_ptr<JITProgram>& GetExecutable() const
        {
            return executable_;
        }

        // Returns the maximum number of viewports that are set in this command buffer.
        inline std::uint32_t GetMaxNumViewports() const
        {
            return maxNumViewports_;
        }

        // Returns the maximum number of scissors that are set in this command buffer.
        inline std::uint32_t GetMaxNumScissors() const
        {
            return maxNumScissors_;
        }

        #endif // /LLGL_ENABLE_JIT_COMPILER

    private:

        void SetGenericBuffer(const GLBufferTarget bufferTarget, Buffer& buffer, std::uint32_t slot);
        void SetGenericBufferArray(const GLBufferTarget bufferTarget, BufferArray& bufferArray, std::uint32_t startSlot);
        void SetResourceHeap(ResourceHeap& resourceHeap);

        /* Allocates only an opcode for empty commands */
        void AllocOpCode(const GLOpcode opcode);

        /* Allocates a new command and stores the specified opcode */
        template <typename T>
        T* AllocCommand(const GLOpcode opcode, std::size_t extraSize = 0);

    private:

        GLRenderState               renderState_;
        GLClearValue                clearValue_;
        GLuint                      boundShaderProgram_ = 0;

        long                        flags_              = 0;
        std::vector<std::uint8_t>   buffer_;

        #ifdef LLGL_ENABLE_JIT_COMPILER
        std::unique_ptr<JITProgram> executable_;
        std::uint32_t               maxNumViewports_    = 0;
        std::uint32_t               maxNumScissors_     = 0;
        #endif // /LLGL_ENABLE_JIT_COMPILER

};


} // /namespace LLGL


#endif



// ================================================================================
