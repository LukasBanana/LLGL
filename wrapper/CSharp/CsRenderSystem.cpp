/*
 * CsRenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsRenderSystem.h"
#include "CsHelper.h"
#include <algorithm>

#if 1//TEST
#include <LLGL/Window.h>
#endif


namespace LHermanns
{

namespace LLGL
{


class UniquePtrContainer
{

    public:

        static ::LLGL::RenderSystem* AddRenderSystem(std::unique_ptr<::LLGL::RenderSystem>&& instance)
        {
            auto ref = instance.get();
            g_renderSystemInstance.emplace_back(std::forward<std::unique_ptr<::LLGL::RenderSystem>&&>(instance));
            return ref;
        }

        static std::unique_ptr<::LLGL::RenderSystem> RemoveRenderSystem(::LLGL::RenderSystem* instance)
        {
            auto it = std::find_if(
                g_renderSystemInstance.begin(),
                g_renderSystemInstance.end(),
                [instance](const std::unique_ptr<::LLGL::RenderSystem>& entry)
                {
                    return (entry.get() == instance);
                }
            );
            if (it != g_renderSystemInstance.end())
            {
                auto instance = std::move(*it);
                g_renderSystemInstance.erase(it);
                return instance;
            }
            else
                return nullptr;
        }

    private:

        static std::vector<std::unique_ptr<::LLGL::RenderSystem>> g_renderSystemInstance;

};

std::vector<std::unique_ptr<::LLGL::RenderSystem>> UniquePtrContainer::g_renderSystemInstance;


RenderSystem::~RenderSystem()
{
    Unload(this);
}

Collections::Generic::List<String^>^ RenderSystem::FindModules()
{
    auto modules = ::LLGL::RenderSystem::FindModules();
    auto modulesManaged = gcnew Collections::Generic::List<String^>();

    for (const auto& moduleName : modules)
        modulesManaged->Add(gcnew String(moduleName.c_str()));

    return modulesManaged;
}

RenderSystem^ RenderSystem::Load(String^ moduleName)
{
    try
    {
        auto instance = ::LLGL::RenderSystem::Load(ToStdString(moduleName));
        return gcnew RenderSystem(std::move(instance));
    }
    catch (const std::exception& e)
    {
        throw gcnew Exception(ToManagedString(e.what()));
    }
}

void RenderSystem::Unload(RenderSystem^ renderSystem)
{
    ::LLGL::RenderSystem::Unload(UniquePtrContainer::RemoveRenderSystem(renderSystem->instance_));
}

int RenderSystem::ID::get()
{
    return instance_->GetRendererID();
}

String^ RenderSystem::Name::get()
{
    return ToManagedString(instance_->GetName());
}

RendererInfo^ RenderSystem::Info::get()
{
    const auto& info = instance_->GetRendererInfo();
    auto managedInfo = gcnew RendererInfo();
    {
        managedInfo->RendererName           = ToManagedString(info.rendererName);
        managedInfo->DeviceName             = ToManagedString(info.deviceName);
        managedInfo->VendorName             = ToManagedString(info.vendorName);
        managedInfo->ShadingLanguageName    = ToManagedString(info.shadingLanguageName);
    }
    return managedInfo;
}

#if 0

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

#endif

/* ----- Render Context ----- */

RenderContext^ RenderSystem::CreateRenderContext(RenderContextDescriptor^ desc)
{
    ::LLGL::RenderContextDescriptor nativeDesc;
    {
        nativeDesc.videoMode.resolution.width   = desc->VideoMode->Resolution->Width;
        nativeDesc.videoMode.resolution.height  = desc->VideoMode->Resolution->Height;
        nativeDesc.videoMode.colorBits          = desc->VideoMode->ColorBits;
        nativeDesc.videoMode.depthBits          = desc->VideoMode->DepthBits;
        nativeDesc.videoMode.stencilBits        = desc->VideoMode->StencilBits;
        nativeDesc.videoMode.fullscreen         = desc->VideoMode->Fullscreen;
        nativeDesc.videoMode.swapChainSize      = desc->VideoMode->SwapChainSize;
    }
    auto renderContext = instance_->CreateRenderContext(nativeDesc);

    #if 1//TEST
    auto& window = static_cast<::LLGL::Window&>(renderContext->GetSurface());
    window.Show(true);
    #endif

    return gcnew RenderContext(renderContext);
}

//RenderContext^ RenderSystem::CreateRenderContext(RenderContextDescriptor^ desc, Surface^ surface);

void RenderSystem::Release(RenderContext^ renderContext)
{
    instance_->Release(*reinterpret_cast<::LLGL::RenderContext*>(renderContext->Native));
}

#if 0

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

RenderSystem::RenderSystem(std::unique_ptr<::LLGL::RenderSystem>&& instance)
{
    instance_ = UniquePtrContainer::AddRenderSystem(
        std::forward<std::unique_ptr<::LLGL::RenderSystem>&&>(instance)
    );
}


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
