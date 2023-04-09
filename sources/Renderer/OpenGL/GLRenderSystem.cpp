/*
 * GLRenderSystem.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLRenderSystem.h"
#include "GLProfile.h"
#include "Texture/GLMipGenerator.h"
#include "Texture/GLTextureViewPool.h"
#include "Ext/GLExtensions.h"
#include "Ext/GLExtensionRegistry.h"
#include "RenderState/GLStatePool.h"
#include "../RenderSystemUtils.h"
#include "GLTypes.h"
#include "GLCore.h"
#include "Shader/GLLegacyShader.h"
#include "Buffer/GLBufferWithVAO.h"
#include "Buffer/GLBufferArrayWithVAO.h"
#include "../CheckedCast.h"
#include "../BufferUtils.h"
#include "../TextureUtils.h"
#include "../../Core/CoreUtils.h"
#include "../../Core/Assertion.h"
#include "GLRenderingCaps.h"
#include "Command/GLImmediateCommandBuffer.h"
#include "Command/GLDeferredCommandBuffer.h"
#include "RenderState/GLGraphicsPSO.h"
#include "RenderState/GLComputePSO.h"
#include <LLGL/Misc/ForRange.h>

#ifdef LLGL_OPENGL
#   include "Shader/GLSeparableShader.h"
#endif


namespace LLGL
{


/* ----- Common ----- */

static RendererConfigurationOpenGL GetGLProfileFromDesc(const RenderSystemDescriptor& renderSystemDesc)
{
    if (auto rendererConfigGL = GetRendererConfiguration<RendererConfigurationOpenGL>(renderSystemDesc))
        return *rendererConfigGL;
    else
        return RendererConfigurationOpenGL{};
}

GLRenderSystem::GLRenderSystem(const RenderSystemDescriptor& renderSystemDesc) :
    contextMngr_ { GetGLProfileFromDesc(renderSystemDesc) }
{
}

GLRenderSystem::~GLRenderSystem()
{
    /* Clear all render state containers first, the rest will be deleted automatically */
    GLTextureViewPool::Get().Clear();
    GLMipGenerator::Get().Clear();
    GLStatePool::Get().Clear();
}

/* ----- Swap-chain ----- */

SwapChain* GLRenderSystem::CreateSwapChain(const SwapChainDescriptor& swapChainDesc, const std::shared_ptr<Surface>& surface)
{
    const bool isFirstSwapChain = swapChains_.empty();
    auto* swapChainGL = swapChains_.emplace<GLSwapChain>(swapChainDesc, surface, contextMngr_);

    /* Create devices that require an active GL context */
    if (isFirstSwapChain)
        CreateGLContextDependentDevices(swapChainGL->GetStateManager());

    return swapChainGL;
}

void GLRenderSystem::Release(SwapChain& swapChain)
{
    swapChains_.erase(&swapChain);
}

/* ----- Command queues ----- */

CommandQueue* GLRenderSystem::GetCommandQueue()
{
    return commandQueue_.get();
}

/* ----- Command buffers ----- */

CommandBuffer* GLRenderSystem::CreateCommandBuffer(const CommandBufferDescriptor& commandBufferDesc)
{
    /* Get state manager from swap-chain with shared GL context */
    if (auto currentGLContext = contextMngr_.AllocContext())
    {
        /* Create deferred or immediate command buffer */
        if ((commandBufferDesc.flags & CommandBufferFlags::ImmediateSubmit) != 0)
            return commandBuffers_.emplace<GLImmediateCommandBuffer>(currentGLContext->GetStateManager());
        else
            return commandBuffers_.emplace<GLDeferredCommandBuffer>(commandBufferDesc.flags);
    }
    else
        LLGL_TRAP("cannot create OpenGL command buffer without active render context");
}

void GLRenderSystem::Release(CommandBuffer& commandBuffer)
{
    commandBuffers_.erase(&commandBuffer);
}

/* ----- Buffers ------ */

static GLbitfield GetGLBufferStorageFlags(long cpuAccessFlags)
{
    #ifdef GL_ARB_buffer_storage

    GLbitfield flagsGL = 0;

    /* Allways enable dynamic storage, to enable usage of 'glBufferSubData' */
    flagsGL |= GL_DYNAMIC_STORAGE_BIT;

    if ((cpuAccessFlags & CPUAccessFlags::Read) != 0)
        flagsGL |= GL_MAP_READ_BIT;
    if ((cpuAccessFlags & CPUAccessFlags::Write) != 0)
        flagsGL |= GL_MAP_WRITE_BIT;

    return flagsGL;

    #else

    return 0;

    #endif // /GL_ARB_buffer_storage
}

static GLenum GetGLBufferUsage(long miscFlags)
{
    return ((miscFlags & MiscFlags::DynamicUsage) != 0 ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
}

static void GLBufferStorage(GLBuffer& bufferGL, const BufferDescriptor& bufferDesc, const void* initialData)
{
    bufferGL.BufferStorage(
        static_cast<GLsizeiptr>(bufferDesc.size),
        initialData,
        GetGLBufferStorageFlags(bufferDesc.cpuAccessFlags),
        GetGLBufferUsage(bufferDesc.miscFlags)
    );
}

Buffer* GLRenderSystem::CreateBuffer(const BufferDescriptor& bufferDesc, const void* initialData)
{
    AssertCreateBuffer(bufferDesc, static_cast<std::uint64_t>(std::numeric_limits<GLsizeiptr>::max()));

    auto bufferGL = CreateGLBuffer(bufferDesc, initialData);

    /* Store meta data for certain types of buffers */
    if ((bufferDesc.bindFlags & BindFlags::IndexBuffer) != 0 && bufferDesc.format != Format::Undefined)
        bufferGL->SetIndexType(bufferDesc.format);

    return bufferGL;
}

// private
GLBuffer* GLRenderSystem::CreateGLBuffer(const BufferDescriptor& bufferDesc, const void* initialData)
{
    /* Create either base of sub-class GLBuffer object */
    if ((bufferDesc.bindFlags & BindFlags::VertexBuffer) != 0)
    {
        /* Create buffer with VAO and build vertex array */
        auto* bufferGL = buffers_.emplace<GLBufferWithVAO>(bufferDesc.bindFlags);
        {
            GLBufferStorage(*bufferGL, bufferDesc, initialData);
            bufferGL->BuildVertexArray(bufferDesc.vertexAttribs.size(), bufferDesc.vertexAttribs.data());
        }
        return bufferGL;
    }
    else
    {
        /* Create generic buffer */
        auto* bufferGL = buffers_.emplace<GLBuffer>(bufferDesc.bindFlags);
        {
            GLBufferStorage(*bufferGL, bufferDesc, initialData);
        }
        return bufferGL;
    }
}

// Returns true if at least one of the buffers in the specified array has a VertexBuffer binding flag.
static bool IsBufferArrayWithVertexBufferBinding(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    for_range(i, numBuffers)
    {
        if ((bufferArray[i]->GetBindFlags() & BindFlags::VertexBuffer) != 0)
            return true;
    }
    return false;
}

BufferArray* GLRenderSystem::CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    AssertCreateBufferArray(numBuffers, bufferArray);

    /* Create vertex buffer array and build VAO if there is at least one buffer with VertexBuffer binding */
    if (IsBufferArrayWithVertexBufferBinding(numBuffers, bufferArray))
        return bufferArrays_.emplace<GLBufferArrayWithVAO>(numBuffers, bufferArray);
    else
        return bufferArrays_.emplace<GLBufferArray>(numBuffers, bufferArray);
}

void GLRenderSystem::Release(Buffer& buffer)
{
    buffers_.erase(&buffer);
}

void GLRenderSystem::Release(BufferArray& bufferArray)
{
    bufferArrays_.erase(&bufferArray);
}

void GLRenderSystem::WriteBuffer(Buffer& buffer, std::uint64_t offset, const void* data, std::uint64_t dataSize)
{
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    bufferGL.BufferSubData(static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(dataSize), data);
}

void GLRenderSystem::ReadBuffer(Buffer& buffer, std::uint64_t offset, void* data, std::uint64_t dataSize)
{
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    bufferGL.GetBufferSubData(static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(dataSize), data);
}

void* GLRenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access)
{
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    return bufferGL.MapBuffer(GLTypes::Map(access));
}

static GLbitfield ToGLMapBufferAccess(CPUAccess access)
{
    switch (access)
    {
        case CPUAccess::ReadOnly:       return GL_MAP_READ_BIT;
        case CPUAccess::WriteOnly:      return GL_MAP_WRITE_BIT;
        case CPUAccess::WriteDiscard:   return GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
        case CPUAccess::ReadWrite:      return GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
        default:                        return 0;
    }
}

void* GLRenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access, std::uint64_t offset, std::uint64_t length)
{
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    return bufferGL.MapBufferRange(static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(length), ToGLMapBufferAccess(access));
}

void GLRenderSystem::UnmapBuffer(Buffer& buffer)
{
    auto& bufferGL = LLGL_CAST(GLBuffer&, buffer);
    bufferGL.UnmapBuffer();
}

/* ----- Textures ----- */

//private
void GLRenderSystem::ValidateGLTextureType(const TextureType type)
{
    /* Validate texture type for this GL device */
    switch (type)
    {
        case TextureType::Texture1D:
        case TextureType::Texture2D:
            break;

        case TextureType::Texture3D:
            LLGL_ASSERT_RENDERING_FEATURE_SUPPORT(has3DTextures);
            break;

        case TextureType::TextureCube:
            LLGL_ASSERT_RENDERING_FEATURE_SUPPORT(hasCubeTextures);
            break;

        case TextureType::Texture1DArray:
        case TextureType::Texture2DArray:
            LLGL_ASSERT_RENDERING_FEATURE_SUPPORT(hasArrayTextures);
            break;

        case TextureType::TextureCubeArray:
            LLGL_ASSERT_RENDERING_FEATURE_SUPPORT(hasCubeArrayTextures);
            break;

        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            LLGL_ASSERT_RENDERING_FEATURE_SUPPORT(hasMultiSampleTextures);
            break;

        default:
            LLGL_TRAP("failed to create texture with invalid texture type");
            break;
    }
}

Texture* GLRenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc)
{
    ValidateGLTextureType(textureDesc.type);

    /* Create <GLTexture> object; will result in a GL renderbuffer or texture instance */
    auto* textureGL = textures_.emplace<GLTexture>(textureDesc);

    /* Initialize either renderbuffer or texture image storage */
    textureGL->BindAndAllocStorage(textureDesc, imageDesc);

    return textureGL;
}

void GLRenderSystem::Release(Texture& texture)
{
    textures_.erase(&texture);
}

void GLRenderSystem::WriteTexture(Texture& texture, const TextureRegion& textureRegion, const SrcImageDescriptor& imageDesc)
{
    /* Bind texture and write texture sub data */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    textureGL.TextureSubImage(textureRegion, imageDesc, false);
}

void GLRenderSystem::ReadTexture(Texture& texture, const TextureRegion& textureRegion, const DstImageDescriptor& imageDesc)
{
    /* Bind texture and write texture sub data */
    LLGL_ASSERT_PTR(imageDesc.data);
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    textureGL.GetTextureSubImage(textureRegion, imageDesc, false);
}

/* ----- Sampler States ---- */

Sampler* GLRenderSystem::CreateSampler(const SamplerDescriptor& samplerDesc)
{
    #ifdef LLGL_GL_ENABLE_OPENGL2X
    if (!HasNativeSamplers())
    {
        /* If GL_ARB_sampler_objects is not supported, use emulated sampler states */
        auto* samplerGL2X = samplersGL2X_.emplace<GL2XSampler>();
        samplerGL2X->SamplerParameters(samplerDesc);
        return samplerGL2X;
    }
    else
    #endif
    {
        /* Create native GL sampler state */
        LLGL_ASSERT_RENDERING_FEATURE_SUPPORT(hasSamplers);
        auto* samplerGL = samplers_.emplace<GLSampler>();
        samplerGL->SamplerParameters(samplerDesc);
        return samplerGL;
    }
}

void GLRenderSystem::Release(Sampler& sampler)
{
    #ifdef LLGL_GL_ENABLE_OPENGL2X
    /* If GL_ARB_sampler_objects is not supported, release emulated sampler states */
    if (!HasNativeSamplers())
        samplersGL2X_.erase(&sampler);
    else
        samplers_.erase(&sampler);
    #else
    samplers_.erase(&sampler);
    #endif
}

/* ----- Resource Heaps ----- */

ResourceHeap* GLRenderSystem::CreateResourceHeap(const ResourceHeapDescriptor& resourceHeapDesc, const ArrayView<ResourceViewDescriptor>& initialResourceViews)
{
    return resourceHeaps_.emplace<GLResourceHeap>(resourceHeapDesc, initialResourceViews);
}

void GLRenderSystem::Release(ResourceHeap& resourceHeap)
{
    resourceHeaps_.erase(&resourceHeap);
}

std::uint32_t GLRenderSystem::WriteResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstDescriptor, const ArrayView<ResourceViewDescriptor>& resourceViews)
{
    auto& resourceHeapGL = LLGL_CAST(GLResourceHeap&, resourceHeap);
    return resourceHeapGL.WriteResourceViews(firstDescriptor, resourceViews);
}

/* ----- Render Passes ----- */

RenderPass* GLRenderSystem::CreateRenderPass(const RenderPassDescriptor& renderPassDesc)
{
    return renderPasses_.emplace<GLRenderPass>(renderPassDesc);
}

void GLRenderSystem::Release(RenderPass& renderPass)
{
    renderPasses_.erase(&renderPass);
}

/* ----- Render Targets ----- */

RenderTarget* GLRenderSystem::CreateRenderTarget(const RenderTargetDescriptor& renderTargetDesc)
{
    LLGL_ASSERT_RENDERING_FEATURE_SUPPORT(hasRenderTargets);
    AssertCreateRenderTarget(renderTargetDesc);
    return renderTargets_.emplace<GLRenderTarget>(renderTargetDesc);
}

void GLRenderSystem::Release(RenderTarget& renderTarget)
{
    renderTargets_.erase(&renderTarget);
}

/* ----- Shader ----- */

Shader* GLRenderSystem::CreateShader(const ShaderDescriptor& shaderDesc)
{
    AssertCreateShader(shaderDesc);

    /* Validate rendering capabilities for required shader type */
    switch (shaderDesc.type)
    {
        case ShaderType::Geometry:
            LLGL_ASSERT_RENDERING_FEATURE_SUPPORT(hasGeometryShaders);
            break;
        case ShaderType::TessControl:
        case ShaderType::TessEvaluation:
            LLGL_ASSERT_RENDERING_FEATURE_SUPPORT(hasTessellationShaders);
            break;
        case ShaderType::Compute:
            LLGL_ASSERT_RENDERING_FEATURE_SUPPORT(hasComputeShaders);
            break;
        default:
            break;
    }

    /* Make and return shader object */
    #ifdef LLGL_OPENGL
    if (HasExtension(GLExt::ARB_separate_shader_objects) && (shaderDesc.flags & ShaderCompileFlags::SeparateShader) != 0)
    {
        /* Create separable shader for program pipeline */
        return shaders_.emplace<GLSeparableShader>(shaderDesc);
    }
    else
    #endif
    {
        /* Create legacy shader for combined program */
        return shaders_.emplace<GLLegacyShader>(shaderDesc);
    }
}

void GLRenderSystem::Release(Shader& shader)
{
    shaders_.erase(&shader);
}

/* ----- Pipeline Layouts ----- */

PipelineLayout* GLRenderSystem::CreatePipelineLayout(const PipelineLayoutDescriptor& pipelineLayoutDesc)
{
    return pipelineLayouts_.emplace<GLPipelineLayout>(pipelineLayoutDesc);
}

void GLRenderSystem::Release(PipelineLayout& pipelineLayout)
{
    pipelineLayouts_.erase(&pipelineLayout);
}

/* ----- Pipeline States ----- */

PipelineState* GLRenderSystem::CreatePipelineState(const Blob& /*serializedCache*/)
{
    return nullptr;//TODO
}

PipelineState* GLRenderSystem::CreatePipelineState(const GraphicsPipelineDescriptor& pipelineStateDesc, std::unique_ptr<Blob>* /*serializedCache*/)
{
    return pipelineStates_.emplace<GLGraphicsPSO>(pipelineStateDesc, GetRenderingCaps().limits);
}

PipelineState* GLRenderSystem::CreatePipelineState(const ComputePipelineDescriptor& pipelineStateDesc, std::unique_ptr<Blob>* /*serializedCache*/)
{
    return pipelineStates_.emplace<GLComputePSO>(pipelineStateDesc);
}

void GLRenderSystem::Release(PipelineState& pipelineState)
{
    pipelineStates_.erase(&pipelineState);
}

/* ----- Queries ----- */

QueryHeap* GLRenderSystem::CreateQueryHeap(const QueryHeapDescriptor& quertHeapDesc)
{
    return queryHeaps_.emplace<GLQueryHeap>(quertHeapDesc);
}

void GLRenderSystem::Release(QueryHeap& queryHeap)
{
    queryHeaps_.erase(&queryHeap);
}

/* ----- Fences ----- */

Fence* GLRenderSystem::CreateFence()
{
    return fences_.emplace<GLFence>();
}

void GLRenderSystem::Release(Fence& fence)
{
    fences_.erase(&fence);
}


/*
 * ======= Private: =======
 */

void GLRenderSystem::CreateGLContextDependentDevices(GLStateManager& stateManager)
{
    /* Enable debug callback function */
    if (debugCallback_)
        SetDebugCallback(debugCallback_);

    /* Create command queue instance */
    commandQueue_ = MakeUnique<GLCommandQueue>(stateManager);

    /* Query renderer information and limits */
    QueryRendererInfo();
    QueryRenderingCaps();
}

#ifdef GL_KHR_debug

void APIENTRY GLDebugCallback(
    GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    /* Generate output stream */
    std::string typeStr;

    typeStr = "OpenGL debug callback (";
    typeStr += GLDebugSourceToStr(source);
    typeStr += ", ";
    typeStr += GLDebugTypeToStr(type);
    typeStr += ", ";
    typeStr += GLDebugSeverityToStr(severity);
    typeStr += ")";

    /* Call debug callback */
    auto debugCallback = reinterpret_cast<const DebugCallback*>(userParam);
    (*debugCallback)(typeStr, message);
}

#endif // /GL_KHR_debug

void GLRenderSystem::SetDebugCallback(const DebugCallback& debugCallback)
{
    #ifdef GL_KHR_debug
    if (HasExtension(GLExt::KHR_debug))
    {
        debugCallback_ = debugCallback;
        if (debugCallback_)
        {
            GLStateManager::Get().Enable(GLState::DEBUG_OUTPUT);
            GLStateManager::Get().Enable(GLState::DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(GLDebugCallback, &debugCallback_);
        }
        else
        {
            GLStateManager::Get().Disable(GLState::DEBUG_OUTPUT);
            GLStateManager::Get().Disable(GLState::DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(nullptr, nullptr);
        }
    }
    #endif // /GL_KHR_debug
}

static std::string GLGetString(GLenum name)
{
    auto bytes = glGetString(name);
    return (bytes != nullptr ? std::string(reinterpret_cast<const char*>(bytes)) : "");
}

void GLRenderSystem::QueryRendererInfo()
{
    RendererInfo info;

    info.rendererName           = GLProfile::GetAPIName() + std::string(" ") + GLGetString(GL_VERSION);
    info.deviceName             = GLGetString(GL_RENDERER);
    info.vendorName             = GLGetString(GL_VENDOR);
    info.shadingLanguageName    = GLProfile::GetShadingLanguageName() + std::string(" ") + GLGetString(GL_SHADING_LANGUAGE_VERSION);

    SetRendererInfo(info);
}

void GLRenderSystem::QueryRenderingCaps()
{
    RenderingCapabilities caps;
    GLQueryRenderingCaps(caps);
    SetRenderingCaps(caps);
}


} // /namespace LLGL



// ================================================================================
