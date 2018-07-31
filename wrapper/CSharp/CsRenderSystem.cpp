/*
 * CsRenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsRenderSystem.h"
#include "CsHelper.h"
#include <algorithm>


namespace SharpLLGL
{


/*
 * Internal classes
 */

class UniquePtrContainer
{

    private:

        template <typename T>
        static T* AddUniqueObject(std::vector<std::unique_ptr<T>>& container, std::unique_ptr<T>&& native)
        {
            auto ref = native.get();
            container.emplace_back(std::forward<std::unique_ptr<T>&&>(native));
            return ref;
        }

        template <typename T>
        static std::unique_ptr<T> RemoveUniqueObject(std::vector<std::unique_ptr<T>>& container, T* native)
        {
            auto it = std::find_if(
                container.begin(),
                container.end(),
                [native](const std::unique_ptr<T>& entry)
                {
                    return (entry.get() == native);
                }
            );
            if (it != container.end())
            {
                auto native = std::move(*it);
                container.erase(it);
                return native;
            }
            else
                return nullptr;
        }

    public:

        /* ----- RenderSystem ----- */

        static LLGL::RenderSystem* AddRenderSystem(std::unique_ptr<LLGL::RenderSystem>&& native)
        {
            return AddUniqueObject(g_renderSystems, std::forward<std::unique_ptr<LLGL::RenderSystem>&&>(native));
        }

        static std::unique_ptr<LLGL::RenderSystem> RemoveRenderSystem(LLGL::RenderSystem* native)
        {
            return RemoveUniqueObject(g_renderSystems, native);
        }

        /* ----- RenderingDebugger ----- */

        static LLGL::RenderingDebugger* AddRenderingDebugger(std::unique_ptr<LLGL::RenderingDebugger>&& native)
        {
            return AddUniqueObject(g_renderingDebuggers, std::forward<std::unique_ptr<LLGL::RenderingDebugger>&&>(native));
        }

        static std::unique_ptr<LLGL::RenderingDebugger> RemoveRenderingDebugger(LLGL::RenderingDebugger* native)
        {
            return RemoveUniqueObject(g_renderingDebuggers, native);
        }

    private:

        static std::vector<std::unique_ptr<LLGL::RenderSystem>>         g_renderSystems;
        static std::vector<std::unique_ptr<LLGL::RenderingDebugger>>    g_renderingDebuggers;

};

std::vector<std::unique_ptr<LLGL::RenderSystem>>        UniquePtrContainer::g_renderSystems;
std::vector<std::unique_ptr<LLGL::RenderingDebugger>>   UniquePtrContainer::g_renderingDebuggers;



/*
 * RenderingDebugger class
 */

RenderingDebugger::RenderingDebugger() :
    native_
    {
        UniquePtrContainer::AddRenderingDebugger(
            std::unique_ptr<LLGL::RenderingDebugger>(
                new LLGL::RenderingDebugger()
            )
        )
    }
{
}

RenderingDebugger::~RenderingDebugger()
{
    UniquePtrContainer::RemoveRenderingDebugger(native_);
}

LLGL::RenderingDebugger* RenderingDebugger::Native::get()
{
    return native_;
}


/*
 * RenderSystem class
 */

RenderSystem::~RenderSystem()
{
    Unload(this);
}

Collections::Generic::List<String^>^ RenderSystem::FindModules()
{
    auto modules = LLGL::RenderSystem::FindModules();
    auto modulesManaged = gcnew Collections::Generic::List<String^>();

    for (const auto& moduleName : modules)
        modulesManaged->Add(gcnew String(moduleName.c_str()));

    return modulesManaged;
}

RenderSystem^ RenderSystem::Load(String^ moduleName)
{
    return Load(moduleName, nullptr);
}

RenderSystem^ RenderSystem::Load(String^ moduleName, RenderingDebugger^ renderingDebugger)
{
    try
    {
        auto native = LLGL::RenderSystem::Load(
            ToStdString(moduleName),
            nullptr,
            (renderingDebugger != nullptr ? renderingDebugger->Native : nullptr)
        );
        return gcnew RenderSystem(std::move(native));
    }
    catch (const std::exception& e)
    {
        throw gcnew Exception(ToManagedString(e.what()));
    }
}

void RenderSystem::Unload(RenderSystem^ renderSystem)
{
    LLGL::RenderSystem::Unload(UniquePtrContainer::RemoveRenderSystem(renderSystem->native_));
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

static void Convert(LLGL::VsyncDescriptor& dst, VsyncDescriptor^ src)
{
    dst.enabled     = src->Enabled;
    dst.refreshRate = src->RefreshRate;
    dst.interval    = src->Interval;
}

static void Convert(LLGL::MultiSamplingDescriptor& dst, MultiSamplingDescriptor^ src)
{
    dst.enabled     = src->Enabled;
    dst.samples     = src->Samples;
    dst.sampleMask  = src->SampleMask;
}

static void Convert(LLGL::VideoModeDescriptor& dst, VideoModeDescriptor^ src)
{
    dst.resolution.width    = src->Resolution->Width;
    dst.resolution.height   = src->Resolution->Height;
    dst.colorBits           = src->ColorBits;
    dst.depthBits           = src->DepthBits;
    dst.stencilBits         = src->StencilBits;
    dst.fullscreen          = src->Fullscreen;
    dst.swapChainSize       = src->SwapChainSize;
}

static void Convert(LLGL::ProfileOpenGLDescriptor& dst, ProfileOpenGLDescriptor^ src)
{
    dst.contextProfile  = static_cast<LLGL::OpenGLContextProfile>(src->ContextProfile);
    dst.majorVersion    = src->MajorVersion;
    dst.minorVersion    = src->MinorVersion;
}

static void Convert(LLGL::RenderContextDescriptor& dst, RenderContextDescriptor^ src)
{
    Convert(dst.vsync, src->Vsync);
    Convert(dst.multiSampling, src->MultiSampling);
    Convert(dst.videoMode, src->VideoMode);
    Convert(dst.profileOpenGL, src->ProfileOpenGL);
    //Convert(dst.debugCallback, src->DebugCallback);
}

RenderContext^ RenderSystem::CreateRenderContext(RenderContextDescriptor^ desc)
{
    LLGL::RenderContextDescriptor nativeDesc;
    Convert(nativeDesc, desc);
    return gcnew RenderContext(native_->CreateRenderContext(nativeDesc));
}

//RenderContext^ RenderSystem::CreateRenderContext(RenderContextDescriptor^ desc, Surface^ surface);

void RenderSystem::Release(RenderContext^ renderContext)
{
    native_->Release(*reinterpret_cast<LLGL::RenderContext*>(renderContext->Native));
}

/* ----- Command queues ----- */

CommandQueue^ RenderSystem::CommandQueue::get()
{
    if (commandQueue_ == nullptr)
        commandQueue_ = gcnew SharpLLGL::CommandQueue(native_->GetCommandQueue());
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

/* ----- Buffers ------ */

static void Convert(LLGL::VertexAttribute& dst, VertexAttribute^ src)
{
    dst.name            = ToStdString(src->Name);
    dst.format          = static_cast<LLGL::Format>(src->Format);
    dst.instanceDivisor = src->InstanceDivisor;
    dst.offset          = src->Offset;
    dst.semanticIndex   = src->SemanticIndex;
}

static void Convert(LLGL::VertexFormat& dst, VertexFormat^ src)
{
    dst.attributes.resize(static_cast<std::size_t>(src->Attributes->Count));
    for (std::size_t i = 0; i < dst.attributes.size(); ++i)
        Convert(dst.attributes[i], src->Attributes[i]);
    dst.stride      = src->Stride;
    dst.inputSlot   = src->InputSlot;
}

static void Convert(LLGL::BufferDescriptor::VertexBuffer& dst, BufferDescriptor::VertexBufferDescriptor^ src)
{
    if (src->Format)
        Convert(dst.format, src->Format);
}

static void Convert(LLGL::IndexFormat& dst, IndexFormat^ src)
{
    dst = LLGL::IndexFormat(static_cast<LLGL::DataType>(src->DataType));
}

static void Convert(LLGL::BufferDescriptor::IndexBuffer& dst, BufferDescriptor::IndexBufferDescriptor^ src)
{
    if (src->Format)
        Convert(dst.format, src->Format);
}

static void Convert(LLGL::BufferDescriptor::StorageBuffer& dst, BufferDescriptor::StorageBufferDescriptor^ src)
{
    //TODO...
}

static void Convert(LLGL::BufferDescriptor& dst, BufferDescriptor^ src)
{
    dst.type    = static_cast<LLGL::BufferType>(src->Type);
    dst.flags   = static_cast<long>(src->Flags);
    dst.size    = src->Size;
    if (src->VertexBuffer)
        Convert(dst.vertexBuffer, src->VertexBuffer);
    if (src->IndexBuffer)
        Convert(dst.indexBuffer, src->IndexBuffer);
    if (src->StorageBuffer)
        Convert(dst.storageBuffer, src->StorageBuffer);
}

Buffer^ RenderSystem::CreateBuffer(BufferDescriptor^ desc)
{
    LLGL::BufferDescriptor nativeDesc;
    Convert(nativeDesc, desc);
    return gcnew Buffer(native_->CreateBuffer(nativeDesc));
}

generic <typename T>
Buffer^ RenderSystem::CreateBuffer(BufferDescriptor^ desc, array<T>^ initialData)
{
    LLGL::BufferDescriptor nativeDesc;
    Convert(nativeDesc, desc);
    pin_ptr<T> initialDataPtr = &initialData[0];
    return gcnew Buffer(native_->CreateBuffer(nativeDesc, initialDataPtr));
}

BufferArray^ RenderSystem::CreateBufferArray(array<Buffer^>^ bufferArray)
{
    if (bufferArray->Length > 0)
    {
        std::vector<LLGL::Buffer*> nativeBufferArray(static_cast<std::size_t>(bufferArray->Length));
        for (std::size_t i = 0; i < nativeBufferArray.size(); ++i)
            nativeBufferArray[i] = bufferArray[i]->NativeSub;
        return gcnew BufferArray(native_->CreateBufferArray(static_cast<std::uint32_t>(nativeBufferArray.size()), nativeBufferArray.data()));
    }
    return nullptr;
}

void RenderSystem::Release(Buffer^ buffer)
{
    native_->Release(*buffer->NativeSub);
}

void RenderSystem::Release(BufferArray^ bufferArray)
{
    native_->Release(*bufferArray->Native);
}

void RenderSystem::WriteBuffer(Buffer^ dstBuffer, System::UInt64 dstOffset, System::IntPtr data, System::UInt64 dataSize)
{
    native_->WriteBuffer(*(dstBuffer->NativeSub), dstOffset, data.ToPointer(), dataSize);
}

System::IntPtr RenderSystem::MapBuffer(Buffer^ buffer, CPUAccess access)
{
    return System::IntPtr(native_->MapBuffer(*(buffer->NativeSub), static_cast<LLGL::CPUAccess>(access)));
}

void RenderSystem::UnmapBuffer(Buffer^ buffer)
{
    native_->UnmapBuffer(*(buffer->NativeSub));
}

/* ----- Textures ----- */

static void Convert(LLGL::Extent3D& dst, Extent3D^ src)
{
    dst.width   = src->Width;
    dst.height  = src->Height;
    dst.depth   = src->Depth;
}

static void Convert(LLGL::TextureDescriptor& dst, TextureDescriptor^ src)
{
    dst.type        = static_cast<LLGL::TextureType>(src->Type);
    dst.flags       = static_cast<long>(src->Flags);
    Convert(dst.extent, src->Extent);
    dst.arrayLayers = src->ArrayLayers;
    dst.mipLevels   = src->MipLevels;
    dst.samples     = src->Samples;
}

Texture^ RenderSystem::CreateTexture(TextureDescriptor^ textureDesc)
{
    LLGL::TextureDescriptor nativeDesc;
    Convert(nativeDesc, textureDesc);
    return gcnew Texture(native_->CreateTexture(nativeDesc));
}

generic <typename T>
Texture^ RenderSystem::CreateTexture(TextureDescriptor^ textureDesc, SrcImageDescriptor<T>^ imageDesc)
{
    LLGL::TextureDescriptor nativeDesc;
    Convert(nativeDesc, textureDesc);

    if (imageDesc != nullptr)
    {
        pin_ptr<T> imageDataRef = &(imageDesc->Data[0]);
        LLGL::SrcImageDescriptor nativeImageDesc;
        {
            nativeImageDesc.format      = static_cast<LLGL::ImageFormat>(imageDesc->Format);
            nativeImageDesc.dataType    = static_cast<LLGL::DataType>(imageDesc->DataType);
            nativeImageDesc.data        = imageDataRef;
            nativeImageDesc.dataSize    = static_cast<std::size_t>(imageDesc->Data->Length) * sizeof(T);
        }
        return gcnew Texture(native_->CreateTexture(nativeDesc, &nativeImageDesc));
    }

    return gcnew Texture(native_->CreateTexture(nativeDesc));
}

void RenderSystem::Release(Texture^ texture)
{
    native_->Release(*texture->NativeSub);
}

#if 0
void RenderSystem::WriteTexture(Texture^ texture, SubTextureDescriptor^ subTextureDesc, SrcImageDescriptor^ imageDesc);
void RenderSystem::ReadTexture(Texture^ texture, unsigned int mipLevel, DstImageDescriptor^ imageDesc);
#endif

void RenderSystem::GenerateMips(Texture^ texture)
{
    native_->GenerateMips(*texture->NativeSub);
}

void RenderSystem::GenerateMips(Texture^ texture, unsigned int baseMipLevel, unsigned int numMipLevels, unsigned int baseArrayLayer, unsigned int numArrayLayers)
{
    native_->GenerateMips(*texture->NativeSub, baseMipLevel, numMipLevels, baseArrayLayer, numArrayLayers);
}

#if 0
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

static void Convert(LLGL::ShaderDescriptor& dst, ShaderDescriptor^ src, std::string (&tempStr)[3])
{
    tempStr[0] = ToStdString(src->Source);
    tempStr[1] = ToStdString(src->EntryPoint);
    tempStr[2] = ToStdString(src->Profile);

    dst.type            = static_cast<LLGL::ShaderType>(src->Type);
    dst.source          = tempStr[0].c_str();
    dst.sourceSize      = tempStr[0].size();
    dst.sourceType      = static_cast<LLGL::ShaderSourceType>(src->SourceType);
    dst.entryPoint      = tempStr[1].c_str();
    dst.profile         = tempStr[2].c_str();
    dst.flags           = src->Flags;
    #if 0
    dst.streamOutput    = ;
    #endif
}

Shader^ RenderSystem::CreateShader(ShaderDescriptor^ desc)
{
    try
    {
        std::string tempStr[3];
        LLGL::ShaderDescriptor nativeDesc;
        Convert(nativeDesc, desc, tempStr);
        return gcnew Shader(native_->CreateShader(nativeDesc));
    }
    catch (const std::exception& e)
    {
        throw gcnew Exception(ToManagedString(e.what()));
    }
}

static void Convert(LLGL::ShaderProgramDescriptor& dst, ShaderProgramDescriptor^ src)
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
    LLGL::ShaderProgramDescriptor nativeDesc;
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

static void Convert(LLGL::Viewport& dst, Viewport^ src)
{
    dst.x           = src->X;
    dst.y           = src->Y;
    dst.width       = src->Width;
    dst.height      = src->Height;
    dst.minDepth    = src->MinDepth;
    dst.maxDepth    = src->MaxDepth;
}

static void Convert(LLGL::Scissor& dst, Scissor^ src)
{
    dst.x       = src->X;
    dst.y       = src->Y;
    dst.width   = src->Width;
    dst.height  = src->Height;
}

static void Convert(LLGL::DepthDescriptor& dst, DepthDescriptor^ src)
{
    dst.testEnabled     = src->TestEnabled;
    dst.writeEnabled    = src->WriteEnabled;
    dst.compareOp       = static_cast<LLGL::CompareOp>(src->CompareOp);
}

static void Convert(LLGL::StencilFaceDescriptor& dst, StencilFaceDescriptor^ src)
{
    dst.stencilFailOp   = static_cast<LLGL::StencilOp>(src->StencilFailOp);
    dst.depthFailOp     = static_cast<LLGL::StencilOp>(src->DepthFailOp);
    dst.depthPassOp     = static_cast<LLGL::StencilOp>(src->DepthPassOp);
    dst.compareOp       = static_cast<LLGL::CompareOp>(src->CompareOp);
    dst.readMask        = src->ReadMask;
    dst.writeMask       = src->WriteMask;
    dst.reference       = src->Reference;
}

static void Convert(LLGL::StencilDescriptor& dst, StencilDescriptor^ src)
{
    dst.testEnabled = src->TestEnabled;
    Convert(dst.front, src->Front);
    Convert(dst.back, src->Back);
}

static void Convert(LLGL::DepthBiasDescriptor& dst, DepthBiasDescriptor^ src)
{
    dst.constantFactor  = src->ConstantFactor;
    dst.slopeFactor     = src->SlopeFactor;
    dst.clamp           = src->Clamp;
}

static void Convert(LLGL::RasterizerDescriptor& dst, RasterizerDescriptor^ src)
{
    dst.polygonMode                 = static_cast<LLGL::PolygonMode>(src->PolygonMode);
    dst.cullMode                    = static_cast<LLGL::CullMode>(src->CullMode);
    Convert(dst.depthBias, src->DepthBias);
    Convert(dst.multiSampling, src->MultiSampling);
    dst.frontCCW                    = src->FrontCCW;
    dst.depthClampEnabled           = src->DepthClampEnabled;
    dst.scissorTestEnabled          = src->ScissorTestEnabled;
    dst.antiAliasedLineEnabled      = src->AntiAliasedLineEnabled;
    dst.conservativeRasterization   = src->ConservativeRasterization;
    dst.lineWidth                   = src->LineWidth;
}

static void Convert(LLGL::BlendTargetDescriptor& dst, BlendTargetDescriptor^ src)
{
    dst.blendEnabled    = src->BlendEnabled;
    dst.srcColor        = static_cast<LLGL::BlendOp>(src->SrcColor);
    dst.dstColor        = static_cast<LLGL::BlendOp>(src->DstColor);
    dst.colorArithmetic = static_cast<LLGL::BlendArithmetic>(src->ColorArithmetic);
    dst.srcAlpha        = static_cast<LLGL::BlendOp>(src->SrcAlpha);
    dst.dstAlpha        = static_cast<LLGL::BlendOp>(src->DstAlpha);
    dst.alphaArithmetic = static_cast<LLGL::BlendArithmetic>(src->AlphaArithmetic);
    for (int i = 0; i < 4; ++i)
        dst.colorMask[i] = (src->ColorMask->Length > i ? src->ColorMask[i] : true);
}

static void Convert(LLGL::BlendDescriptor& dst, BlendDescriptor^ src)
{
    for (int i = 0; i < 4; ++i)
        dst.blendFactor[i] = (src->BlendFactor->Length > i ? src->BlendFactor[i] : 0.0f);
    dst.alphaToCoverageEnabled  = src->AlphaToCoverageEnabled;
    dst.independentBlendEnabled = src->IndependentBlendEnabled;
    dst.logicOp                 = static_cast<LLGL::LogicOp>(src->LogicOp);
    for (int i = 0, n = std::min(8, src->Targets->Length); i < n; ++i)
        Convert(dst.targets[i], src->Targets[i]);
}

static void Convert(LLGL::GraphicsPipelineDescriptor& dst, GraphicsPipelineDescriptor^ src)
{
    dst.shaderProgram       = (src->ShaderProgram != nullptr ? src->ShaderProgram->Native : nullptr);
    dst.renderPass          = (src->RenderPass != nullptr ? src->RenderPass->Native : nullptr);
    dst.pipelineLayout      = (src->PipelineLayout != nullptr ? src->PipelineLayout->Native : nullptr);
    dst.primitiveTopology   = static_cast<LLGL::PrimitiveTopology>(src->PrimitiveTopology);

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
    LLGL::GraphicsPipelineDescriptor nativeDesc;
    Convert(nativeDesc, desc);
    try
    {
        return gcnew GraphicsPipeline(native_->CreateGraphicsPipeline(nativeDesc));
    }
    catch (const std::exception& e)
    {
        throw gcnew Exception(ToManagedString(e.what()));
    }
}

#if 0
ComputePipeline^ RenderSystem::CreateComputePipeline(ComputePipelineDescriptor^ desc);
#endif

void RenderSystem::Release(GraphicsPipeline^ graphicsPipeline)
{
    native_->Release(*graphicsPipeline->Native);
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

RenderSystem::RenderSystem(std::unique_ptr<LLGL::RenderSystem>&& native)
{
    native_ = UniquePtrContainer::AddRenderSystem(
        std::forward<std::unique_ptr<LLGL::RenderSystem>&&>(native)
    );
}


} // /namespace SharpLLGL



// ================================================================================
