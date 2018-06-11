/*
 * CsRenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/RenderSystem.h>
#include <vcclr.h>

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;


namespace LLGL
{


public ref class RenderSystem
{

    public:

        /* ----- Common ----- */

        static std::vector<std::string> FindModules();

        static RenderSystem^ Load(
            /*const RenderSystemDescriptor& renderSystemDesc,
            RenderingProfiler^ profiler = nullptr,
            RenderingDebugger^ debugger = nullptr*/)
        {
            return gcnew RenderSystem();
        }

        static void Unload(std::unique_ptr<RenderSystem>&& renderSystem);

        #if 0

        int GetRendererID()
        {
            return rendererID_;
        }

        String^ GetName()
        {
            return name_;
        }

        RendererInfo GetRendererInfo()
        {
            return info_;
        }

        RenderingCapabilities GetRenderingCaps()
        {
            return caps_;
        }

        void SetConfiguration(const RenderSystemConfiguration& config)
        {
        }

        const RenderSystemConfiguration& GetConfiguration()
        {
            return config_;
        }

        /* ----- Render Context ----- */

        RenderContext^ CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Surface>& surface);

        void Release(RenderContext^ renderContext);

        /* ----- Command queues ----- */

        CommandQueue^ GetCommandQueue()

        /* ----- Command buffers ----- */

        CommandBuffer^ CreateCommandBuffer();

        CommandBufferExt^ CreateCommandBufferExt();

        void Release(CommandBuffer& commandBuffer);

        /* ----- Buffers ------ */

        Buffer^ CreateBuffer(const BufferDescriptor& desc, const void* initialData);

        BufferArray^ CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray);

        void Release(Buffer& buffer);

        void Release(BufferArray& bufferArray);

        void WriteBuffer(Buffer& buffer, const void* data, std::size_t dataSize, std::size_t offset);

        void* MapBuffer(Buffer& buffer, const CPUAccess access);

        void UnmapBuffer(Buffer& buffer);

        /* ----- Textures ----- */

        Texture^ CreateTexture(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc);

        void Release(Texture& texture);

        void WriteTexture(Texture& texture, const SubTextureDescriptor& subTextureDesc, const SrcImageDescriptor& imageDesc);

        void ReadTexture(const Texture& texture, std::uint32_t mipLevel, const DstImageDescriptor& imageDesc);

        void GenerateMips(Texture& texture);

        void GenerateMips(Texture& texture, std::uint32_t baseMipLevel, std::uint32_t numMipLevels, std::uint32_t baseArrayLayer, std::uint32_t numArrayLayers);

        /* ----- Samplers ---- */

        Sampler* CreateSampler(const SamplerDescriptor& desc);

        void Release(Sampler& sampler);

        /* ----- Resource Heaps ----- */

        ResourceHeap^ CreateResourceHeap(const ResourceHeapDescriptor& desc);

        void Release(ResourceHeap& resourceHeap);

        /* ----- Render Targets ----- */

        RenderTarget^ CreateRenderTarget(const RenderTargetDescriptor& desc);

        void Release(RenderTarget& renderTarget);

        /* ----- Shader ----- */

        Shader^ CreateShader(const ShaderType type);

        ShaderProgram^ CreateShaderProgram();

        void Release(Shader& shader);

        void Release(ShaderProgram& shaderProgram);

        /* ----- Pipeline Layouts ----- */

        PipelineLayout^ CreatePipelineLayout(const PipelineLayoutDescriptor& desc);

        void Release(PipelineLayout& pipelineLayout);

        /* ----- Pipeline States ----- */

        GraphicsPipeline^ CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc);

        ComputePipeline^ CreateComputePipeline(const ComputePipelineDescriptor& desc);

        void Release(GraphicsPipeline& graphicsPipeline);

        void Release(ComputePipeline& computePipeline);

        /* ----- Queries ----- */

        Query* CreateQuery(const QueryDescriptor& desc);

        void Release(Query& query);

        /* ----- Fences ----- */

        Fence^ CreateFence();

        void Release(Fence& fence);

        #endif

    private:



};


} // /namespace LLGL



// ================================================================================
