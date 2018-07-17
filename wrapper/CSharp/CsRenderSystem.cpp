/*
 * CsRenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsRenderSystem.h"
#include "CsHelper.h"
#include <algorithm>

#include <iostream>

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

static void Convert(::LLGL::ShaderDescriptor& dst, ShaderDescriptor^ src, std::string (&tempStr)[3])
{
    tempStr[0] = ToStdString(src->Source);
    tempStr[1] = ToStdString(src->EntryPoint);
    tempStr[2] = ToStdString(src->Profile);

    dst.type            = static_cast<::LLGL::ShaderType>(src->Type);
    dst.source          = tempStr[0].c_str();
    dst.sourceSize      = tempStr[0].size();
    dst.sourceType      = static_cast<::LLGL::ShaderSourceType>(src->SourceType);
    dst.entryPoint      = tempStr[1].c_str();
    dst.profile         = tempStr[2].c_str();
    dst.flags           = src->Flags;
    #if 0
    dst.streamOutput    = ;
    #endif
}

Shader^ RenderSystem::CreateShader(ShaderDescriptor^ desc)
{
    std::string tempStr[3];
    ::LLGL::ShaderDescriptor nativeDesc;
    Convert(nativeDesc, desc, tempStr);
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
#endif

/* ----- Pipeline States ----- */

static void Convert(::LLGL::Viewport& dst, Viewport^ src)
{
    dst.x           = src->X;
    dst.y           = src->Y;
    dst.width       = src->Width;
    dst.height      = src->Height;
    dst.minDepth    = src->MinDepth;
    dst.maxDepth    = src->MaxDepth;
}

static void Convert(::LLGL::Scissor& dst, Scissor^ src)
{
    dst.x       = src->X;
    dst.y       = src->Y;
    dst.width   = src->Width;
    dst.height  = src->Height;
}

static void Convert(::LLGL::MultiSamplingDescriptor& dst, MultiSamplingDescriptor^ src)
{
    dst.enabled = src->Enabled;
    dst.samples = src->Samples;
}

static void Convert(::LLGL::DepthDescriptor& dst, DepthDescriptor^ src)
{
    dst.testEnabled     = src->TestEnabled;
    dst.writeEnabled    = src->WriteEnabled;
    dst.compareOp       = static_cast<::LLGL::CompareOp>(src->CompareOp);
}

static void Convert(::LLGL::StencilFaceDescriptor& dst, StencilFaceDescriptor^ src)
{
    dst.stencilFailOp   = static_cast<::LLGL::StencilOp>(src->StencilFailOp);
    dst.depthFailOp     = static_cast<::LLGL::StencilOp>(src->DepthFailOp);
    dst.depthPassOp     = static_cast<::LLGL::StencilOp>(src->DepthPassOp);
    dst.compareOp       = static_cast<::LLGL::CompareOp>(src->CompareOp);
    dst.readMask        = src->ReadMask;
    dst.writeMask       = src->WriteMask;
    dst.reference       = src->Reference;
}

static void Convert(::LLGL::StencilDescriptor& dst, StencilDescriptor^ src)
{
    dst.testEnabled = src->TestEnabled;
    Convert(dst.front, src->Front);
    Convert(dst.back, src->Back);
}

static void Convert(::LLGL::DepthBiasDescriptor& dst, DepthBiasDescriptor^ src)
{
    dst.constantFactor  = src->ConstantFactor;
    dst.slopeFactor     = src->SlopeFactor;
    dst.clamp           = src->Clamp;
}

static void Convert(::LLGL::RasterizerDescriptor& dst, RasterizerDescriptor^ src)
{
    dst.polygonMode                 = static_cast<::LLGL::PolygonMode>(src->PolygonMode);
    dst.cullMode                    = static_cast<::LLGL::CullMode>(src->CullMode);
    Convert(dst.depthBias, src->DepthBias);
    Convert(dst.multiSampling, src->MultiSampling);
    dst.frontCCW                    = src->FrontCCW;
    dst.depthClampEnabled           = src->DepthClampEnabled;
    dst.scissorTestEnabled          = src->ScissorTestEnabled;
    dst.antiAliasedLineEnabled      = src->AntiAliasedLineEnabled;
    dst.conservativeRasterization   = src->ConservativeRasterization;
    dst.lineWidth                   = src->LineWidth;
}

static void Convert(::LLGL::BlendTargetDescriptor& dst, BlendTargetDescriptor^ src)
{
    dst.srcColor        = static_cast<::LLGL::BlendOp>(src->SrcColor);
    dst.dstColor        = static_cast<::LLGL::BlendOp>(src->DstColor);
    dst.colorArithmetic = static_cast<::LLGL::BlendArithmetic>(src->ColorArithmetic);
    dst.srcAlpha        = static_cast<::LLGL::BlendOp>(src->SrcAlpha);
    dst.dstAlpha        = static_cast<::LLGL::BlendOp>(src->DstAlpha);
    dst.alphaArithmetic = static_cast<::LLGL::BlendArithmetic>(src->AlphaArithmetic);
    for (int i = 0; i < 4; ++i)
        dst.colorMask[i] = (src->ColorMask->Length > i ? src->ColorMask[i] : true);
}

static void Convert(::LLGL::BlendDescriptor& dst, BlendDescriptor^ src)
{
    dst.blendEnabled            = src->BlendEnabled;
    for (int i = 0; i < 4; ++i)
        dst.blendFactor[i] = (src->BlendFactor->Length > i ? src->BlendFactor[i] : 0.0f);
    dst.alphaToCoverageEnabled  = src->AlphaToCoverageEnabled;
    dst.logicOp                 = static_cast<::LLGL::LogicOp>(src->LogicOp);
    dst.targets.resize(src->Targets->Count);
    for (std::size_t i = 0; i < dst.targets.size(); ++i)
        Convert(dst.targets[i], src->Targets[i]);
}

static void Convert(::LLGL::GraphicsPipelineDescriptor& dst, GraphicsPipelineDescriptor^ src)
{
    dst.shaderProgram       = (src->ShaderProgram != nullptr ? src->ShaderProgram->Native : nullptr);
    dst.renderPass          = (src->RenderPass != nullptr ? src->RenderPass->Native : nullptr);
    dst.pipelineLayout      = (src->PipelineLayout != nullptr ? src->PipelineLayout->Native : nullptr);
    dst.primitiveTopology   = static_cast<::LLGL::PrimitiveTopology>(src->PrimitiveTopology);

    dst.viewports.resize(src->Viewports->Count);
    for (std::size_t i = 0; i < dst.viewports.size(); ++i)
        Convert(dst.viewports[i], src->Viewports[i]);

    dst.scissors.resize(src->Scissors->Count);
    for (std::size_t i = 0; i < dst.scissors.size(); ++i)
        Convert(dst.scissors[i], src->Scissors[i]);

    Convert(dst.depth, src->Depth);
    Convert(dst.stencil, src->Stencil);
    Convert(dst.rasterizer, src->Rasterizer);
    Convert(dst.blend, src->Blend);
}

GraphicsPipeline^ RenderSystem::CreateGraphicsPipeline(GraphicsPipelineDescriptor^ desc)
{
    ::LLGL::GraphicsPipelineDescriptor nativeDesc;
    Convert(nativeDesc, desc);
    return gcnew GraphicsPipeline(native_->CreateGraphicsPipeline(nativeDesc));
}

#if 0
ComputePipeline^ RenderSystem::CreateComputePipeline(ComputePipelineDescriptor^ desc);
#endif

void RenderSystem::Release(GraphicsPipeline^ graphicsPipeline)
{
}

#if 0
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
