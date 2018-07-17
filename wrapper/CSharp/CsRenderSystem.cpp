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

static void Convert(::LLGL::RenderContextDescriptor& dst, RenderContextDescriptor^ src)
{
    dst.videoMode.resolution.width   = src->VideoMode->Resolution->Width;
    dst.videoMode.resolution.height  = src->VideoMode->Resolution->Height;
    dst.videoMode.colorBits          = src->VideoMode->ColorBits;
    dst.videoMode.depthBits          = src->VideoMode->DepthBits;
    dst.videoMode.stencilBits        = src->VideoMode->StencilBits;
    dst.videoMode.fullscreen         = src->VideoMode->Fullscreen;
    dst.videoMode.swapChainSize      = src->VideoMode->SwapChainSize;
}

RenderContext^ RenderSystem::CreateRenderContext(RenderContextDescriptor^ desc)
{
    ::LLGL::RenderContextDescriptor nativeDesc;
    Convert(nativeDesc, desc);
    return gcnew RenderContext(native_->CreateRenderContext(nativeDesc));
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
    native_->Release(*commandBuffer->Native);
}

#if 0

/* ----- Buffers ------ */

Buffer^ RenderSystem::CreateBuffer(BufferDescriptor^ desc);
Buffer^ RenderSystem::CreateBuffer(BufferDescriptor^ desc, array<System::Byte>^ initialData);

BufferArray^ RenderSystem::CreateBufferArray(array<Buffer^>^ bufferArray);

void RenderSystem::Release(Buffer^ buffer);

void RenderSystem::Release(BufferArray^ bufferArray);

void RenderSystem::WriteBuffer(Buffer^ buffer, array<System::Byte>^ data, System::UIntPtr dataSize, System::UIntPtr offset);

//void* RenderSystem::MapBuffer(Buffer^ buffer, CPUAccess access);

//void RenderSystem::UnmapBuffer(Buffer^ buffer);

/* ----- Textures ----- */

Texture^ RenderSystem::CreateTexture(TextureDescriptor^ textureDesc);
Texture^ RenderSystem::CreateTexture(TextureDescriptor^ textureDesc, SrcImageDescriptor^ imageDesc);

void RenderSystem::Release(Texture^ texture);

void RenderSystem::WriteTexture(Texture^ texture, SubTextureDescriptor^ subTextureDesc, SrcImageDescriptor^ imageDesc);
void RenderSystem::ReadTexture(Texture^ texture, unsigned int mipLevel, DstImageDescriptor^ imageDesc);

void RenderSystem::GenerateMips(Texture^ texture);
void RenderSystem::GenerateMips(Texture^ texture, unsigned int baseMipLevel, unsigned int numMipLevels, unsigned int baseArrayLayer, unsigned int numArrayLayers);

/* ----- Samplers ---- */

Sampler^ RenderSystem::CreateSampler(SamplerDescriptor^ desc);

void RenderSystem::Release(Sampler^ sampler);

/* ----- Resource Heaps ----- */

ResourceHeap^ RenderSystem::CreateResourceHeap(ResourceHeapDescriptor^ desc);

void RenderSystem::Release(ResourceHeap^ resourceHeap);

/* ----- Render Targets ----- */

RenderTarget^ RenderSystem::CreateRenderTarget(RenderTargetDescriptor^ desc);

void RenderSystem::Release(RenderTarget^ renderTarget);
#endif

/* ----- Shader ----- */

static void Convert(::LLGL::ShaderDescriptor& dst, ShaderDescriptor^ src)
{
    auto sourceStr      = ToStdString(src->Source);
    auto entryPointStr  = ToStdString(src->EntryPoint);
    auto profileStr     = ToStdString(src->Profile);

    dst.type            = static_cast<::LLGL::ShaderType>(src->Type);
    dst.source          = sourceStr.c_str();
    dst.sourceSize      = sourceStr.size();
    dst.sourceType      = static_cast<::LLGL::ShaderSourceType>(src->SourceType);
    dst.entryPoint      = entryPointStr.c_str();
    dst.profile         = profileStr.c_str();
    dst.flags           = src->Flags;
    #if 0
    dst.streamOutput    = ;
    #endif
}

Shader^ RenderSystem::CreateShader(ShaderDescriptor^ desc)
{
    ::LLGL::ShaderDescriptor nativeDesc;
    Convert(nativeDesc, desc);
    return gcnew Shader(native_->CreateShader(nativeDesc));
}

static void Convert(::LLGL::VertexAttribute& dst, VertexAttribute^ src)
{
    dst.name            = ToStdString(src->Name);
    dst.format          = static_cast<::LLGL::Format>(src->Format);
    dst.instanceDivisor = src->InstanceDivisor;
    dst.offset          = src->Offset;
    dst.semanticIndex   = src->SemanticIndex;
}

static void Convert(::LLGL::VertexFormat& dst, VertexFormat^ src)
{
    dst.attributes.resize(static_cast<std::size_t>(src->Attributes->Count));
    for (std::size_t i = 0; i < dst.attributes.size(); ++i)
        Convert(dst.attributes[i], src->Attributes[i]);
    dst.stride      = src->Stride;
    dst.inputSlot   = src->InputSlot;
}

static void Convert(::LLGL::ShaderProgramDescriptor& dst, ShaderProgramDescriptor^ src)
{
    dst.vertexFormats.resize(static_cast<std::size_t>(src->VertexFormats->Count));
    for (std::size_t i = 0; i < dst.vertexFormats.size(); ++i)
        Convert(dst.vertexFormats[i], src->VertexFormats[i]);
    if (src->VertexShader)
        dst.vertexShader = src->VertexShader->Native;
    if (src->TessControlShader)
        dst.tessControlShader = src->TessControlShader->Native;
    if (src->TessEvaluationShader)
        dst.tessEvaluationShader = src->TessEvaluationShader->Native;
    if (src->GeometryShader)
        dst.geometryShader = src->GeometryShader->Native;
    if (src->FragmentShader)
        dst.fragmentShader = src->FragmentShader->Native;
    if (src->ComputeShader)
        dst.computeShader = src->ComputeShader->Native;
}

ShaderProgram^ RenderSystem::CreateShaderProgram(ShaderProgramDescriptor^ desc)
{
    ::LLGL::ShaderProgramDescriptor nativeDesc;
    Convert(nativeDesc, desc);
    return gcnew ShaderProgram(native_->CreateShaderProgram(nativeDesc));
}

void RenderSystem::Release(Shader^ shader)
{
    native_->Release(*shader->Native);
}

void RenderSystem::Release(ShaderProgram^ shaderProgram)
{
    native_->Release(*shaderProgram->Native);
}

#if 0
/* ----- Pipeline Layouts ----- */

PipelineLayout^ RenderSystem::CreatePipelineLayout(PipelineLayoutDescriptor^ desc);

void RenderSystem::Release(PipelineLayout^ pipelineLayout);

/* ----- Pipeline States ----- */

GraphicsPipeline^ RenderSystem::CreateGraphicsPipeline(GraphicsPipelineDescriptor^ desc);

ComputePipeline^ RenderSystem::CreateComputePipeline(ComputePipelineDescriptor^ desc);

void RenderSystem::Release(GraphicsPipeline^ graphicsPipeline);

void RenderSystem::Release(ComputePipeline^ computePipeline);

#endif

/* ----- Fences ----- */

Fence^ RenderSystem::CreateFence()
{
    return gcnew Fence(native_->CreateFence());
}

void RenderSystem::Release(Fence^ fence)
{
    native_->Release(*fence->Native);
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
