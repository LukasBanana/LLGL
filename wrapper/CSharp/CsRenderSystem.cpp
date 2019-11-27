/*
 * CsRenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsRenderSystem.h"
#include "CsHelper.h"
#include <LLGL/ImageFlags.h>
#include <LLGL/Utility.h>
#include <algorithm>


namespace SharpLLGL
{


/*
 * Common converions
 */

static void Convert(LLGL::ColorRGBAf& dst, ColorRGBA<float>^ src)
{
    if (src)
    {
        dst.r = src->R;
        dst.g = src->G;
        dst.b = src->B;
        dst.a = src->A;
    }
}

static void Convert(LLGL::ColorRGBAb& dst, ColorRGBA<bool>^ src)
{
    if (src)
    {
        dst.r = src->R;
        dst.g = src->G;
        dst.b = src->B;
        dst.a = src->A;
    }
}

static void Convert(LLGL::Extent2D& dst, Extent2D^ src)
{
    if (src)
    {
        dst.width   = src->Width;
        dst.height  = src->Height;
    }
}

static void Convert(LLGL::Extent3D& dst, Extent3D^ src)
{
    if (src)
    {
        dst.width   = src->Width;
        dst.height  = src->Height;
        dst.depth   = src->Depth;
    }
}


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
    if (src)
    {
        dst.enabled     = src->Enabled;
        dst.refreshRate = src->RefreshRate;
        dst.interval    = src->Interval;
    }
}

static void Convert(LLGL::VideoModeDescriptor& dst, VideoModeDescriptor^ src)
{
    if (src)
    {
        dst.resolution.width    = src->Resolution->Width;
        dst.resolution.height   = src->Resolution->Height;
        dst.colorBits           = src->ColorBits;
        dst.depthBits           = src->DepthBits;
        dst.stencilBits         = src->StencilBits;
        dst.fullscreen          = src->Fullscreen;
        dst.swapChainSize       = src->SwapChainSize;
    }
}

static void Convert(LLGL::RendererConfigurationOpenGL& dst, RendererConfigurationOpenGL^ src)
{
    if (src)
    {
        dst.contextProfile  = static_cast<LLGL::OpenGLContextProfile>(src->ContextProfile);
        dst.majorVersion    = src->MajorVersion;
        dst.minorVersion    = src->MinorVersion;
    }
}

static void Convert(LLGL::RenderContextDescriptor& dst, RenderContextDescriptor^ src)
{
    if (src)
    {
        Convert(dst.vsync, src->Vsync);
        dst.samples = src->Samples;
        Convert(dst.videoMode, src->VideoMode);
    }
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

void RenderSystem::Release(CommandBuffer^ commandBuffer)
{
    native_->Release(*commandBuffer->Native);
}

/* ----- Buffers ------ */

static void Convert(LLGL::VertexAttribute& dst, VertexAttribute^ src)
{
    if (src)
    {
        dst.name            = ToStdString(src->Name);
        dst.format          = static_cast<LLGL::Format>(src->Format);
        dst.location        = src->Location;
        dst.semanticIndex   = src->SemanticIndex;
        dst.slot            = src->Slot;
        dst.offset          = src->Offset;
        dst.stride          = src->Stride;
        dst.instanceDivisor = src->InstanceDivisor;
    }
}

static void Convert(LLGL::BufferDescriptor& dst, BufferDescriptor^ src)
{
    if (src)
    {
        dst.size            = src->Size;
        dst.stride          = src->Stride;
        dst.format          = static_cast<LLGL::Format>(src->Format);
        dst.bindFlags       = static_cast<long>(src->BindFlags);
        dst.cpuAccessFlags  = static_cast<long>(src->CPUAccessFlags);
        dst.miscFlags       = static_cast<long>(src->MiscFlags);
        dst.vertexAttribs.resize(src->VertexAttribs->Count);
        for (int i = 0; i < src->VertexAttribs->Count; ++i)
            Convert(dst.vertexAttribs[i], src->VertexAttribs[i]);
    }
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
            nativeBufferArray[i] = bufferArray[static_cast<int>(i)]->NativeSub;
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

static void Convert(LLGL::TextureDescriptor& dst, TextureDescriptor^ src)
{
    if (src)
    {
        dst.type            = static_cast<LLGL::TextureType>(src->Type);
        dst.bindFlags       = static_cast<long>(src->BindFlags);
        dst.miscFlags       = static_cast<long>(src->MiscFlags);
        Convert(dst.extent, src->Extent);
        dst.arrayLayers     = src->ArrayLayers;
        dst.mipLevels       = src->MipLevels;
        dst.samples         = src->Samples;
    }
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

/* ----- Samplers ---- */

static void Convert(LLGL::SamplerDescriptor& dst, SamplerDescriptor^ src)
{
    if (src)
    {
        dst.addressModeU    = static_cast<LLGL::SamplerAddressMode>(src->AddressModeU);
        dst.addressModeV    = static_cast<LLGL::SamplerAddressMode>(src->AddressModeV);
        dst.addressModeW    = static_cast<LLGL::SamplerAddressMode>(src->AddressModeW);
        dst.minFilter       = static_cast<LLGL::SamplerFilter>(src->MinFilter);
        dst.magFilter       = static_cast<LLGL::SamplerFilter>(src->MagFilter);
        dst.mipMapFilter    = static_cast<LLGL::SamplerFilter>(src->MipMapFilter);
        dst.mipMapping      = src->MipMapping;
        dst.mipMapLODBias   = src->MipMapLODBias;
        dst.minLOD          = src->MinLOD;
        dst.maxLOD          = src->MaxLOD;
        dst.maxAnisotropy   = src->MaxAnisotropy;
        dst.compareEnabled  = src->CompareEnabled;
        dst.compareOp       = static_cast<LLGL::CompareOp>(src->CompareOp);
        Convert(dst.borderColor, src->BorderColor);
    }
}

Sampler^ RenderSystem::CreateSampler(SamplerDescriptor^ desc)
{
    LLGL::SamplerDescriptor nativeDesc;
    Convert(nativeDesc, desc);
    return gcnew Sampler(native_->CreateSampler(nativeDesc));
}

void RenderSystem::Release(Sampler^ sampler)
{
    native_->Release(*sampler->NativeSub);
}

/* ----- Resource Heaps ----- */

static void Convert(LLGL::ResourceViewDescriptor& dst, ResourceViewDescriptor^ src)
{
    if (src)
        dst.resource = src->Resource->Native;
}

static void Convert(LLGL::ResourceHeapDescriptor& dst, ResourceHeapDescriptor^ src)
{
    if (src)
    {
        dst.pipelineLayout = src->PipelineLayout->Native;
        dst.resourceViews.resize(src->ResourceViews->Count);
        for (std::size_t i = 0; i < dst.resourceViews.size(); ++i)
            Convert(dst.resourceViews[i], src->ResourceViews[i]);
    }
}

ResourceHeap^ RenderSystem::CreateResourceHeap(ResourceHeapDescriptor^ desc)
{
    LLGL::ResourceHeapDescriptor nativeDesc;
    Convert(nativeDesc, desc);
    return gcnew ResourceHeap(native_->CreateResourceHeap(nativeDesc));
}

void RenderSystem::Release(ResourceHeap^ resourceHeap)
{
    native_->Release(*resourceHeap->Native);
}

/* ----- Render Targets ----- */

static void Convert(LLGL::AttachmentDescriptor& dst, AttachmentDescriptor^ src)
{
    if (src)
    {
        dst.type        = static_cast<LLGL::AttachmentType>(src->Type);
        dst.texture     = (src->Texture != nullptr ? src->Texture->NativeSub : nullptr);
        dst.mipLevel    = src->MipLevel;
        dst.arrayLayer  = src->ArrayLayer;
    }
}

static void Convert(LLGL::RenderTargetDescriptor& dst, RenderTargetDescriptor^ src)
{
    if (src)
    {
        dst.renderPass = (src->RenderPass != nullptr ? src->RenderPass->Native : nullptr);
        Convert(dst.resolution, src->Resolution);
        dst.samples = src->Samples;
        dst.customMultiSampling = src->CustomMultiSampling;
        dst.attachments.resize(src->Attachments->Count);
        for (std::size_t i = 0; i < dst.attachments.size(); ++i)
            Convert(dst.attachments[i], src->Attachments[i]);
    }
}

RenderTarget^ RenderSystem::CreateRenderTarget(RenderTargetDescriptor^ desc)
{
    LLGL::RenderTargetDescriptor nativeDesc;
    Convert(nativeDesc, desc);
    return gcnew RenderTarget(native_->CreateRenderTarget(nativeDesc));
}

void RenderSystem::Release(RenderTarget^ renderTarget)
{
    native_->Release(*renderTarget->Native);
}

/* ----- Shader ----- */

static void Convert(LLGL::VertexShaderAttributes& dst, VertexShaderAttributes^ src)
{
    dst.inputAttribs.resize(src->InputAttribs->Count);
    for (int i = 0; i < src->InputAttribs->Count; ++i)
        Convert(dst.inputAttribs[i], src->InputAttribs[i]);

    dst.outputAttribs.resize(src->OutputAttribs->Count);
    for (int i = 0; i < src->OutputAttribs->Count; ++i)
        Convert(dst.outputAttribs[i], src->OutputAttribs[i]);
}

/*
static void Convert(LLGL::FragmentShaderAttributes& dst, FragmentShaderAttributes^ src)
{
    dst.outputAttribs.resize(src->OutputAttribs->Count);
    for (int i = 0; i < src->InputAttribs->Count; ++i)
        Convert(dst.outputAttribs[i], src->OutputAttribs[i]);
}
*/

static void Convert(LLGL::ShaderDescriptor& dst, ShaderDescriptor^ src, std::string (&tempStr)[3])
{
    if (src)
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
        dst.flags           = static_cast<long>(src->Flags);
        Convert(dst.vertex, src->Vertex);
        //Convert(dst.fragment, src->Fragment);
    }
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
    if (src)
    {
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

/* ----- Pipeline Layouts ----- */

static void Convert(LLGL::BindingDescriptor& dst, BindingDescriptor^ src)
{
    if (src)
    {
        dst.type        = static_cast<LLGL::ResourceType>(src->Type);
        dst.bindFlags   = static_cast<long>(src->BindFlags);
        dst.stageFlags  = static_cast<long>(src->StageFlags);
        dst.slot        = src->Slot;
        dst.arraySize   = src->ArraySize;
        dst.name        = ToStdString(src->Name);
    }
}

static void Convert(LLGL::PipelineLayoutDescriptor& dst, PipelineLayoutDescriptor^ src)
{
    if (src)
    {
        dst.bindings.resize(src->Bindings->Count);
        for (std::size_t i = 0; i < dst.bindings.size(); ++i)
            Convert(dst.bindings[i], src->Bindings[i]);
    }
}

PipelineLayout^ RenderSystem::CreatePipelineLayout(PipelineLayoutDescriptor^ desc)
{
    LLGL::PipelineLayoutDescriptor nativeDesc;
    Convert(nativeDesc, desc);
    return gcnew PipelineLayout(native_->CreatePipelineLayout(nativeDesc));
}

PipelineLayout^ RenderSystem::CreatePipelineLayout(String^ layoutSignature)
{
    auto layoutSignatureCStr = ToStdString(layoutSignature);
    LLGL::PipelineLayoutDescriptor nativeDesc = LLGL::PipelineLayoutDesc(layoutSignatureCStr.c_str());
    return gcnew PipelineLayout(native_->CreatePipelineLayout(nativeDesc));
}

void RenderSystem::Release(PipelineLayout^ pipelineLayout)
{
    native_->Release(*pipelineLayout->Native);
}

/* ----- Pipeline States ----- */

static void Convert(LLGL::Viewport& dst, Viewport^ src)
{
    if (src)
    {
        dst.x           = src->X;
        dst.y           = src->Y;
        dst.width       = src->Width;
        dst.height      = src->Height;
        dst.minDepth    = src->MinDepth;
        dst.maxDepth    = src->MaxDepth;
    }
}

static void Convert(LLGL::Scissor& dst, Scissor^ src)
{
    if (src)
    {
        dst.x       = src->X;
        dst.y       = src->Y;
        dst.width   = src->Width;
        dst.height  = src->Height;
    }
}

static void Convert(LLGL::DepthDescriptor& dst, DepthDescriptor^ src)
{
    if (src)
    {
        dst.testEnabled     = src->TestEnabled;
        dst.writeEnabled    = src->WriteEnabled;
        dst.compareOp       = static_cast<LLGL::CompareOp>(src->CompareOp);
    }
}

static void Convert(LLGL::StencilFaceDescriptor& dst, StencilFaceDescriptor^ src)
{
    if (src)
    {
        dst.stencilFailOp   = static_cast<LLGL::StencilOp>(src->StencilFailOp);
        dst.depthFailOp     = static_cast<LLGL::StencilOp>(src->DepthFailOp);
        dst.depthPassOp     = static_cast<LLGL::StencilOp>(src->DepthPassOp);
        dst.compareOp       = static_cast<LLGL::CompareOp>(src->CompareOp);
        dst.readMask        = src->ReadMask;
        dst.writeMask       = src->WriteMask;
        dst.reference       = src->Reference;
    }
}

static void Convert(LLGL::StencilDescriptor& dst, StencilDescriptor^ src)
{
    if (src)
    {
        dst.testEnabled         = src->TestEnabled;
        dst.referenceDynamic    = src->ReferenceDynamic;
        Convert(dst.front, src->Front);
        Convert(dst.back, src->Back);
    }
}

static void Convert(LLGL::DepthBiasDescriptor& dst, DepthBiasDescriptor^ src)
{
    if (src)
    {
        dst.constantFactor  = src->ConstantFactor;
        dst.slopeFactor     = src->SlopeFactor;
        dst.clamp           = src->Clamp;
    }
}

static void Convert(LLGL::RasterizerDescriptor& dst, RasterizerDescriptor^ src)
{
    if (src)
    {
        dst.polygonMode                 = static_cast<LLGL::PolygonMode>(src->PolygonMode);
        dst.cullMode                    = static_cast<LLGL::CullMode>(src->CullMode);
        Convert(dst.depthBias, src->DepthBias);
        dst.multiSampleEnabled          = src->MultiSampleEnabled;
        dst.frontCCW                    = src->FrontCCW;
        dst.discardEnabled              = src->DiscardEnabled;
        dst.depthClampEnabled           = src->DepthClampEnabled;
        dst.scissorTestEnabled          = src->ScissorTestEnabled;
        dst.antiAliasedLineEnabled      = src->AntiAliasedLineEnabled;
        dst.conservativeRasterization   = src->ConservativeRasterization;
        dst.lineWidth                   = src->LineWidth;
    }
}

static void Convert(LLGL::BlendTargetDescriptor& dst, BlendTargetDescriptor^ src)
{
    if (src)
    {
        dst.blendEnabled    = src->BlendEnabled;
        dst.srcColor        = static_cast<LLGL::BlendOp>(src->SrcColor);
        dst.dstColor        = static_cast<LLGL::BlendOp>(src->DstColor);
        dst.colorArithmetic = static_cast<LLGL::BlendArithmetic>(src->ColorArithmetic);
        dst.srcAlpha        = static_cast<LLGL::BlendOp>(src->SrcAlpha);
        dst.dstAlpha        = static_cast<LLGL::BlendOp>(src->DstAlpha);
        dst.alphaArithmetic = static_cast<LLGL::BlendArithmetic>(src->AlphaArithmetic);
        Convert(dst.colorMask, src->ColorMask);
    }
}

static void Convert(LLGL::BlendDescriptor& dst, BlendDescriptor^ src)
{
    if (src)
    {
        Convert(dst.blendFactor, src->BlendFactor);
        dst.blendFactorDynamic      = src->BlendFactorDynamic;
        dst.alphaToCoverageEnabled  = src->AlphaToCoverageEnabled;
        dst.independentBlendEnabled = src->IndependentBlendEnabled;
        dst.logicOp                 = static_cast<LLGL::LogicOp>(src->LogicOp);
        for (int i = 0, n = std::min(8, src->Targets->Length); i < n; ++i)
            Convert(dst.targets[i], src->Targets[i]);
    }
}

static void Convert(LLGL::GraphicsPipelineDescriptor& dst, GraphicsPipelineDescriptor^ src)
{
    if (src)
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
}

PipelineState^ RenderSystem::CreatePipelineState(GraphicsPipelineDescriptor^ desc)
{
    LLGL::GraphicsPipelineDescriptor nativeDesc;
    Convert(nativeDesc, desc);
    try
    {
        return gcnew PipelineState(native_->CreatePipelineState(nativeDesc));
    }
    catch (const std::exception& e)
    {
        throw gcnew Exception(ToManagedString(e.what()));
    }
}

#if 0
ComputePipeline^ RenderSystem::CreateComputePipeline(ComputePipelineDescriptor^ desc);
#endif

void RenderSystem::Release(PipelineState^ pipelineState)
{
    native_->Release(*pipelineState->Native);
}

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
