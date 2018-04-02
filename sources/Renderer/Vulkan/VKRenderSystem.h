/*
 * VKRenderSystem.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_RENDER_SYSTEM_H
#define LLGL_VK_RENDER_SYSTEM_H


#include <LLGL/RenderSystem.h>
#include "Vulkan.h"
#include "VKPtr.h"
#include "../ContainerTypes.h"

#include "VKCommandQueue.h"
#include "VKCommandBuffer.h"
#include "VKRenderContext.h"

#include "Buffer/VKBuffer.h"
#include "Buffer/VKBufferArray.h"

#include "Shader/VKShader.h"
#include "Shader/VKShaderProgram.h"

//#include "Texture/VKTexture.h"
//#include "Texture/VKTextureArray.h"
#include "Texture/VKSampler.h"
#include "Texture/VKSamplerArray.h"
//#include "Texture/VKRenderTarget.h"

#include "RenderState/VKQuery.h"
#include "RenderState/VKFence.h"
#include "RenderState/VKPipelineLayout.h"
#include "RenderState/VKGraphicsPipeline.h"
//#include "RenderState/VKComputePipeline.h"

#include <string>
#include <memory>
#include <vector>
#include <set>


namespace LLGL
{


class VKRenderSystem : public RenderSystem
{

    public:

        /* ----- Common ----- */

        VKRenderSystem();
        ~VKRenderSystem();

        /* ----- Render Context ----- */

        RenderContext* CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Surface>& surface = nullptr) override;

        void Release(RenderContext& renderContext) override;

        /* ----- Command queues ----- */

        CommandQueue* GetCommandQueue() override;

        /* ----- Command buffers ----- */

        CommandBuffer* CreateCommandBuffer() override;

        void Release(CommandBuffer& commandBuffer) override;

        /* ----- Buffers ------ */

        Buffer* CreateBuffer(const BufferDescriptor& desc, const void* initialData = nullptr) override;
        BufferArray* CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray) override;

        void Release(Buffer& buffer) override;
        void Release(BufferArray& bufferArray) override;
        
        void WriteBuffer(Buffer& buffer, const void* data, std::size_t dataSize, std::size_t offset) override;

        void* MapBuffer(Buffer& buffer, const BufferCPUAccess access) override;
        void UnmapBuffer(Buffer& buffer) override;

        /* ----- Textures ----- */

        Texture* CreateTexture(const TextureDescriptor& textureDesc, const ImageDescriptor* imageDesc = nullptr) override;
        TextureArray* CreateTextureArray(std::uint32_t numTextures, Texture* const * textureArray) override;

        void Release(Texture& texture) override;
        void Release(TextureArray& textureArray) override;

        TextureDescriptor QueryTextureDescriptor(const Texture& texture) override;

        void WriteTexture(Texture& texture, const SubTextureDescriptor& subTextureDesc, const ImageDescriptor& imageDesc) override;
        
        void ReadTexture(const Texture& texture, std::uint32_t mipLevel, ImageFormat imageFormat, DataType dataType, void* data, std::size_t dataSize) override;

        void GenerateMips(Texture& texture) override;

        /* ----- Sampler States ---- */

        Sampler* CreateSampler(const SamplerDescriptor& desc) override;
        SamplerArray* CreateSamplerArray(std::uint32_t numSamplers, Sampler* const * samplerArray) override;

        void Release(Sampler& sampler) override;
        void Release(SamplerArray& samplerArray) override;

        /* ----- Render Targets ----- */

        RenderTarget* CreateRenderTarget(const RenderTargetDescriptor& desc) override;

        void Release(RenderTarget& renderTarget) override;

        /* ----- Shader ----- */

        Shader* CreateShader(const ShaderType type) override;
        ShaderProgram* CreateShaderProgram() override;

        void Release(Shader& shader) override;
        void Release(ShaderProgram& shaderProgram) override;
        
        /* ----- Pipeline Layouts ----- */

        PipelineLayout* CreatePipelineLayout(const PipelineLayoutDescriptor& desc) override;

        void Release(PipelineLayout& pipelineLayout) override;

        /* ----- Pipeline States ----- */

        GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc) override;
        ComputePipeline* CreateComputePipeline(const ComputePipelineDescriptor& desc) override;
        
        void Release(GraphicsPipeline& graphicsPipeline) override;
        void Release(ComputePipeline& computePipeline) override;

        /* ----- Queries ----- */

        Query* CreateQuery(const QueryDescriptor& desc) override;

        void Release(Query& query) override;

        /* ----- Fences ----- */

        Fence* CreateFence() override;

        void Release(Fence& fence) override;

    private:

        void CreateInstance(const ApplicationDescriptor& appDesc);
        void CreateDebugReportCallback();
        void LoadExtensions();
        bool PickPhysicalDevice();
        void QueryDeviceProperties();
        void CreateLogicalDevice();

        void CreateStagingCommandResources();
        void ReleaseStagingCommandResources();

        bool IsLayerRequired(const std::string& name) const;
        bool IsExtensionRequired(const std::string& name) const;
        bool IsPhysicalDeviceSuitable(VkPhysicalDevice device) const;
        bool CheckDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& extensionNames) const;

        std::uint32_t FindMemoryType(std::uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

        VKBuffer* CreateHardwareBuffer(const BufferDescriptor& desc, VkBufferUsageFlags usage = 0);

        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        /* ----- Common objects ----- */

        VKPtr<VkInstance>                       instance_;
        VkPhysicalDevice                        physicalDevice_         = VK_NULL_HANDLE;
        VKPtr<VkDevice>                         device_;
        VKPtr<VkDebugReportCallbackEXT>         debugReportCallback_;

        QueueFamilyIndices                      queueFamilyIndices_;
        VkPhysicalDeviceMemoryProperties        memoryPropertiers_;
        VkPhysicalDeviceFeatures                features_;

        VkQueue                                 graphicsQueue_          = VK_NULL_HANDLE;

        VKPtr<VkCommandPool>                    stagingCommandPool_;
        VkCommandBuffer                         stagingCommandBuffer_   = VK_NULL_HANDLE;

        bool                                    debugLayerEnabled_      = false;

        /* ----- Hardware object containers ----- */

        HWObjectContainer<VKRenderContext>      renderContexts_;
        HWObjectInstance<VKCommandQueue>        commandQueue_;
        HWObjectContainer<VKCommandBuffer>      commandBuffers_;
        HWObjectContainer<VKBuffer>             buffers_;
        HWObjectContainer<VKBufferArray>        bufferArrays_;
        /*HWObjectContainer<VKTexture>            textures_;
        HWObjectContainer<VKTextureArray>       textureArrays_;*/
        HWObjectContainer<VKSampler>            samplers_;
        HWObjectContainer<VKSamplerArray>       samplerArrays_;
        //HWObjectContainer<VKRenderTarget>       renderTargets_;
        HWObjectContainer<VKShader>             shaders_;
        HWObjectContainer<VKShaderProgram>      shaderPrograms_;
        HWObjectContainer<VKPipelineLayout>     pipelineLayouts_;
        HWObjectContainer<VKGraphicsPipeline>   graphicsPipelines_;
        //HWObjectContainer<VKComputePipeline>    computePipelines_;
        HWObjectContainer<VKQuery>              queries_;
        HWObjectContainer<VKFence>              fences_;

};


} // /namespace LLGL


#endif



// ================================================================================
