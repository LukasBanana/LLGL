/*
 * VKCommandBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_COMMAND_BUFFER_H
#define LLGL_VK_COMMAND_BUFFER_H


#include <LLGL/CommandBuffer.h>
#include "Vulkan.h"
#include "VKPtr.h"
#include "VKCore.h"

#include <vector>


namespace LLGL
{


class VKCommandBuffer : public CommandBuffer
{

    public:

        /* ----- Common ----- */

        VKCommandBuffer(const VKPtr<VkDevice>& device, std::size_t bufferCount, const QueueFamilyIndices& queueFamilyIndices);
        ~VKCommandBuffer();

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

        /* ----- Buffers ------ */

        void SetVertexBuffer(Buffer& buffer) override;
        void SetVertexBufferArray(BufferArray& bufferArray) override;

        void SetIndexBuffer(Buffer& buffer) override;
        
        void SetConstantBuffer(Buffer& buffer, std::uint32_t slot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        void SetConstantBufferArray(BufferArray& bufferArray, std::uint32_t startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        
        void SetStorageBuffer(Buffer& buffer, std::uint32_t slot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        void SetStorageBufferArray(BufferArray& bufferArray, std::uint32_t startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) override;

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

        /* ----- Misc ----- */

        void SyncGPU() override;

        /* --- Extended functions --- */

        void SetPresentIndex(std::uint32_t idx);

        void BeginCommandBuffer();
        void EndCommandBuffer();

        void BeginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, const VkExtent2D& extent);
        void EndRenderPass();

        inline VkCommandBuffer GetBufferObject() const
        {
            return commandBuffer_;
        }

    private:

        void CreateCommandPool(std::uint32_t queueFamilyIndex);
        void CreateCommandBuffers(std::size_t bufferCount);

        void BeginClearImage(
            VkImageMemoryBarrier& clearToPresentBarrier, VkImage image,
            const VkImageAspectFlags clearFlags, const VkClearColorValue* clearColor, const VkClearDepthStencilValue* clearDepthStencil
        );
        void EndClearImage(VkImageMemoryBarrier& clearToPresentBarrier);

        VkDevice                        device_;
        VKPtr<VkCommandPool>            commandPool_;
        std::vector<VkCommandBuffer>    commandBufferList_;
        VkCommandBuffer                 commandBuffer_;

        VkClearColorValue               clearColor_         = { 0.0f, 0.0f, 0.0f, 0.0f };
        VkClearDepthStencilValue        clearDepthStencil_  = { 1.0f, 0 };

        VkRenderPass                    renderPass_         = VK_NULL_HANDLE;
        VkFramebuffer                   framebuffer_        = VK_NULL_HANDLE;
        VkImage                         imageColor_         = VK_NULL_HANDLE;
        VkImage                         imageDepthStencil_  = VK_NULL_HANDLE;

        std::uint32_t                   queuePresentFamily_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
