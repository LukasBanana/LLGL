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

        static ::LLGL::RenderSystem* AddRenderSystem(std::unique_ptr<::LLGL::RenderSystem>&& native)
        {
            auto ref = native.get();
            g_renderSystemInstance.emplace_back(std::forward<std::unique_ptr<::LLGL::RenderSystem>&&>(native));
            return ref;
        }

        static std::unique_ptr<::LLGL::RenderSystem> RemoveRenderSystem(::LLGL::RenderSystem* native)
        {
            auto it = std::find_if(
                g_renderSystemInstance.begin(),
                g_renderSystemInstance.end(),
                [native](const std::unique_ptr<::LLGL::RenderSystem>& entry)
                {
                    return (entry.get() == native);
                }
            );
            if (it != g_renderSystemInstance.end())
            {
                auto native = std::move(*it);
                g_renderSystemInstance.erase(it);
                return native;
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
        auto native = ::LLGL::RenderSystem::Load(ToStdString(moduleName));
        return gcnew RenderSystem(std::move(native));
    }
    catch (const std::exception& e)
    {
        throw gcnew Exception(ToManagedString(e.what()));
    }
}

void RenderSystem::Unload(RenderSystem^ renderSystem)
{
    ::LLGL::RenderSystem::Unload(UniquePtrContainer::RemoveRenderSystem(renderSystem->native_));
}

int RenderSystem::ID::get()
{
    return native_->GetRendererID();
}

String^ RenderSystem::Name::get()
{
    return ToManagedString(native_->GetName());
}

RendererInfo^ RenderSystem::Info::get()
{
    const auto& info = native_->GetRendererInfo();
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
    auto renderContext = native_->CreateRenderContext(nativeDesc);

    return gcnew RenderContext(renderContext);
}

//RenderContext^ RenderSystem::CreateRenderContext(RenderContextDescriptor^ desc, Surface^ surface);

void RenderSystem::Release(RenderContext^ renderContext)
{
    native_->Release(*reinterpret_cast<::LLGL::RenderContext*>(renderContext->Native));
}

/* ----- Command queues ----- */

CommandQueue^ RenderSystem::CommandQueue::get()
{
    if (commandQueue_ == nullptr)
        commandQueue_ = gcnew LHermanns::LLGL::CommandQueue(native_->GetCommandQueue());
    return commandQueue_;
}

/* ----- Command buffers ----- */

CommandBuffer^ RenderSystem::CreateCommandBuffer()
{
    return gcnew CommandBuffer(native_->CreateCommandBuffer());
}

#if 0
CommandBufferExt^ RenderSystem::CreateCommandBufferExt()
{
}
#endif

void RenderSystem::Release(CommandBuffer^ commandBuffer)
{
    native_->Release(*commandBuffer->Native::get());
}

#if 0

/* ----- Buffers ------ */

Buffer^ CreateBuffer(BufferDescriptor^ desc);
Buffer^ CreateBuffer(BufferDescriptor^ desc, array<System::Byte>^ initialData);

//BufferArray^ CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray);

void Release(Buffer^ buffer);

//void Release(BufferArray^ bufferArray);

void WriteBuffer(Buffer^ buffer, array<System::Byte>^ data, System::UIntPtr dataSize, System::UIntPtr offset);

//void* MapBuffer(Buffer^ buffer, CPUAccess access);

//void UnmapBuffer(Buffer^ buffer);

/* ----- Textures ----- */

Texture^ CreateTexture(TextureDescriptor^ textureDesc);
Texture^ CreateTexture(TextureDescriptor^ textureDesc, SrcImageDescriptor^ imageDesc);

void Release(Texture^ texture);

void WriteTexture(Texture^ texture, SubTextureDescriptor^ subTextureDesc, SrcImageDescriptor^ imageDesc);
void ReadTexture(Texture^ texture, unsigned int mipLevel, DstImageDescriptor^ imageDesc);

void GenerateMips(Texture^ texture);
void GenerateMips(Texture^ texture, unsigned int baseMipLevel, unsigned int numMipLevels, unsigned int baseArrayLayer, unsigned int numArrayLayers);

/* ----- Samplers ---- */

Sampler^ CreateSampler(SamplerDescriptor^ desc);

void Release(Sampler^ sampler);

/* ----- Resource Heaps ----- */

ResourceHeap^ CreateResourceHeap(ResourceHeapDescriptor^ desc);

void Release(ResourceHeap^ resourceHeap);

/* ----- Render Targets ----- */

RenderTarget^ CreateRenderTarget(RenderTargetDescriptor^ desc);

void Release(RenderTarget^ renderTarget);

/* ----- Shader ----- */

Shader^ CreateShader(ShaderDescriptor^ desc);

ShaderProgram^ CreateShaderProgram(ShaderProgramDescriptor^ desc);

void Release(Shader^ shader);

void Release(ShaderProgram^ shaderProgram);

/* ----- Pipeline Layouts ----- */

PipelineLayout^ CreatePipelineLayout(PipelineLayoutDescriptor^ desc);

void Release(PipelineLayout^ pipelineLayout);

/* ----- Pipeline States ----- */

GraphicsPipeline^ CreateGraphicsPipeline(GraphicsPipelineDescriptor^ desc);

ComputePipeline^ CreateComputePipeline(ComputePipelineDescriptor^ desc);

void Release(GraphicsPipeline^ graphicsPipeline);

void Release(ComputePipeline^ computePipeline);

#endif

/* ----- Fences ----- */

Fence^ RenderSystem::CreateFence()
{
    return gcnew Fence(native_->CreateFence());
}

void RenderSystem::Release(Fence^ fence)
{
    native_->Release(*fence->Native::get());
}

RenderSystem::RenderSystem(std::unique_ptr<::LLGL::RenderSystem>&& native)
{
    native_ = UniquePtrContainer::AddRenderSystem(
        std::forward<std::unique_ptr<::LLGL::RenderSystem>&&>(native)
    );
}


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
