/*
 * GLCommandBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_COMMAND_BUFFER_H
#define LLGL_GL_COMMAND_BUFFER_H


#include <LLGL/CommandBufferExt.h>
#include "RenderState/GLState.h"
#include "OpenGL.h"


namespace LLGL
{


class GLRenderTarget;
class GLStateManager;

class GLCommandBuffer : public CommandBufferExt
{

    public:

        /* ----- Common ----- */

        GLCommandBuffer(const std::shared_ptr<GLStateManager>& stateManager);

        /* ----- Configuration ----- */

        void SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state) override;

        void SetViewport(const Viewport& viewport) override;
        void SetViewportArray(std::uint32_t numViewports, const Viewport* viewportArray) override;

        void SetScissor(const Scissor& scissor) override;
        void SetScissorArray(std::uint32_t numScissors, const Scissor* scissorArray) override;

        void SetClearColor(const ColorRGBAf& color) override;
        void SetClearDepth(float depth) override;
        void SetClearStencil(std::uint32_t stencil) override;

        void Clear(long flags) override;
        void ClearTarget(std::uint32_t targetIndex, const LLGL::ColorRGBAf& color) override;

        /* ----- Input Assembly ------ */

        void SetVertexBuffer(Buffer& buffer) override;
        void SetVertexBufferArray(BufferArray& bufferArray) override;

        void SetIndexBuffer(Buffer& buffer) override;
        
        /* ----- Constant Buffers ------ */

        void SetConstantBuffer(Buffer& buffer, std::uint32_t slot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        void SetConstantBufferArray(BufferArray& bufferArray, std::uint32_t startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        
        /* ----- Storage Buffers ------ */

        void SetStorageBuffer(Buffer& buffer, std::uint32_t slot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        void SetStorageBufferArray(BufferArray& bufferArray, std::uint32_t startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) override;

        /* ----- Stream Output Buffers ------ */

        void SetStreamOutputBuffer(Buffer& buffer) override;
        void SetStreamOutputBufferArray(BufferArray& bufferArray) override;

        void BeginStreamOutput(const PrimitiveType primitiveType) override;
        void EndStreamOutput() override;

        /* ----- Textures ----- */

        void SetTexture(Texture& texture, std::uint32_t layer, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        void SetTextureArray(TextureArray& textureArray, std::uint32_t startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) override;

        /* ----- Sampler States ----- */

        void SetSampler(Sampler& sampler, std::uint32_t layer, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        void SetSamplerArray(SamplerArray& samplerArray, std::uint32_t startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) override;

        /* ----- Render Targets ----- */

        void SetRenderTarget(RenderTarget& renderTarget) override;
        void SetRenderTarget(RenderContext& renderContext) override;

        /* ----- Pipeline States ----- */

        void SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline) override;
        void SetComputePipeline(ComputePipeline& computePipeline) override;

        /* ----- Queries ----- */

        void BeginQuery(Query& query) override;
        void EndQuery(Query& query) override;

        bool QueryResult(Query& query, std::uint64_t& result) override;
        bool QueryPipelineStatisticsResult(Query& query, QueryPipelineStatistics& result) override;

        void BeginRenderCondition(Query& query, const RenderConditionMode mode) override;
        void EndRenderCondition() override;

        /* ----- Drawing ----- */

        void Draw(std::uint32_t numVertices, std::uint32_t firstVertex) override;

        void DrawIndexed(std::uint32_t numVertices, std::uint32_t firstIndex) override;
        void DrawIndexed(std::uint32_t numVertices, std::uint32_t firstIndex, std::int32_t vertexOffset) override;

        void DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances) override;
        void DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t instanceOffset) override;

        void DrawIndexedInstanced(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex) override;
        void DrawIndexedInstanced(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset) override;
        void DrawIndexedInstanced(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t instanceOffset) override;

        /* ----- Compute ----- */

        void Dispatch(std::uint32_t groupSizeX, std::uint32_t groupSizeY, std::uint32_t groupSizeZ) override;

    private:

        struct RenderState
        {
            GLenum      drawMode            = GL_TRIANGLES;     // Render mode for "glDraw*"
            GLenum      indexBufferDataType = GL_UNSIGNED_INT;
            GLsizeiptr  indexBufferStride   = 4;
        };

        void SetGenericBuffer(const GLBufferTarget bufferTarget, Buffer& buffer, std::uint32_t slot);
        void SetGenericBufferArray(const GLBufferTarget bufferTarget, BufferArray& bufferArray, std::uint32_t startSlot);

        // Blits the currently bound render target
        void BlitBoundRenderTarget();

        std::shared_ptr<GLStateManager> stateMngr_;
        RenderState                     renderState_;

        GLRenderTarget*                 boundRenderTarget_  = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
