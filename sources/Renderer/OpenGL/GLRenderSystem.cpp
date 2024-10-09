/*
 * GLRenderSystem.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLRenderSystem.h"
#include "Profile/GLProfile.h"
#include "Texture/GLMipGenerator.h"
#include "Texture/GLTextureViewPool.h"
#include "Texture/GLFramebufferCapture.h"
#include "Ext/GLExtensions.h"
#include "Ext/GLExtensionRegistry.h"
#include "RenderState/GLStatePool.h"
#include "../RenderSystemUtils.h"
#include "GLTypes.h"
#include "GLCore.h"
#include "Shader/GLLegacyShader.h"
#include "Buffer/GLBufferWithVAO.h"
#include "Buffer/GLBufferWithXFB.h"
#include "Buffer/GLBufferArrayWithVAO.h"
#include "../CheckedCast.h"
#include "../BufferUtils.h"
#include "../TextureUtils.h"
#include "../RenderTargetUtils.h"
#include "../../Core/CoreUtils.h"
#include "../../Core/Assertion.h"
#include "../../Platform/Debug.h"
#include "GLRenderingCaps.h"
#include "Ext/GLExtensionLoader.h"
#include "Command/GLImmediateCommandBuffer.h"
#include "Command/GLDeferredCommandBuffer.h"
#include "RenderState/GLGraphicsPSO.h"
#include "RenderState/GLComputePSO.h"
#include <LLGL/Utils/ForRange.h>

#ifdef LLGL_OPENGL
#   include "Shader/GLSeparableShader.h"
#endif


namespace LLGL
{


/* ----- Common ----- */

static RendererConfigurationOpenGL GetGLProfileFromDesc(const RenderSystemDescriptor& renderSystemDesc)
{
    if (auto* rendererConfigGL = GetRendererConfiguration<RendererConfigurationOpenGL>(renderSystemDesc))
        return *rendererConfigGL;
    else
        return RendererConfigurationOpenGL{};
}

GLRenderSystem::GLRenderSystem(const RenderSystemDescriptor& renderSystemDesc) :
    contextMngr_
    {
        GetGLProfileFromDesc(renderSystemDesc),
        std::bind(&GLRenderSystem::RegisterNewGLContext, this, std::placeholders::_1, std::placeholders::_2),
        renderSystemDesc.nativeHandle,
        renderSystemDesc.nativeHandleSize
    },
    debugContext_
    {
        ((renderSystemDesc.flags & RenderSystemFlags::DebugDevice) != 0)
    }
{
}

GLRenderSystem::~GLRenderSystem()
{
    /* Clear all render state containers first, the rest will be deleted automatically */
    GLFramebufferCapture::Get().Clear();
    GLTextureViewPool::Get().Clear();
    GLMipGenerator::Get().Clear();
    GLStatePool::Get().Clear();
}

/* ----- Swap-chain ----- */

SwapChain* GLRenderSystem::CreateSwapChain(const SwapChainDescriptor& swapChainDesc, const std::shared_ptr<Surface>& surface)
{
    return swapChains_.emplace<GLSwapChain>(*this, swapChainDesc, surface, contextMngr_);
}

void GLRenderSystem::Release(SwapChain& swapChain)
{
    swapChains_.erase(&swapChain);
}

/* ----- Command queues ----- */

CommandQueue* GLRenderSystem::GetCommandQueue()
{
    return &commandQueue_;
}

/* ----- Command buffers ----- */

CommandBuffer* GLRenderSystem::CreateCommandBuffer(const CommandBufferDescriptor& commandBufferDesc)
{
    /* Create deferred or immediate command buffer */
    CreateGLContextOnce();
    if ((commandBufferDesc.flags & CommandBufferFlags::ImmediateSubmit) != 0)
        return commandBuffers_.emplace<GLImmediateCommandBuffer>();
    else
        return commandBuffers_.emplace<GLDeferredCommandBuffer>(commandBufferDesc.flags);
}

void GLRenderSystem::Release(CommandBuffer& commandBuffer)
{
    commandBuffers_.erase(&commandBuffer);
}

/* ----- Buffers ------ */

static GLbitfield GetGLBufferStorageFlags(long cpuAccessFlags)
{
    #if GL_ARB_buffer_storage

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
    CreateGLContextOnce();
    RenderSystem::AssertCreateBuffer(bufferDesc, static_cast<std::uint64_t>(std::numeric_limits<GLsizeiptr>::max()));

    auto bufferGL = CreateGLBuffer(bufferDesc, initialData);

    /* Store meta data for certain types of buffers */
    if ((bufferDesc.bindFlags & BindFlags::IndexBuffer) != 0 && bufferDesc.format != Format::Undefined)
        bufferGL->SetIndexType(bufferDesc.format);

    return bufferGL;
}

// private
GLBuffer* GLRenderSystem::CreateGLBuffer(const BufferDescriptor& bufferDesc, const void* initialData)
{
    #if LLGL_GLEXT_TRNASFORM_FEEDBACK2
    if ((bufferDesc.bindFlags & BindFlags::StreamOutputBuffer) != 0)
    {
        /* Create buffer with VAO and transform feedback object */
        auto* bufferGL = buffers_.emplace<GLBufferWithXFB>(bufferDesc.bindFlags, bufferDesc.debugName);
        {
            GLBufferStorage(*bufferGL, bufferDesc, initialData);
            bufferGL->BuildVertexArray(bufferDesc.vertexAttribs);
        }
        return bufferGL;
    }
    else
    #endif // /LLGL_GLEXT_TRNASFORM_FEEDBACK2
    if ((bufferDesc.bindFlags & BindFlags::VertexBuffer) != 0)
    {
        /* Create buffer with VAO and build vertex array */
        auto* bufferGL = buffers_.emplace<GLBufferWithVAO>(bufferDesc.bindFlags, bufferDesc.debugName);
        {
            GLBufferStorage(*bufferGL, bufferDesc, initialData);
            bufferGL->BuildVertexArray(bufferDesc.vertexAttribs);
        }
        return bufferGL;
    }
    else
    {
        /* Create generic buffer */
        auto* bufferGL = buffers_.emplace<GLBuffer>(bufferDesc.bindFlags, bufferDesc.debugName);
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
    CreateGLContextOnce();
    RenderSystem::AssertCreateBufferArray(numBuffers, bufferArray);

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

    #if LLGL_GLEXT_MEMORY_BARRIERS
    if ((bufferGL.GetBindFlags() & BindFlags::Storage) != 0)
    {
        /* Ensure all shader writes to the buffer completed */
        if (HasExtension(GLExt::ARB_shader_image_load_store))
            glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    }
    #endif // /LLGL_GLEXT_MEMORY_BARRIERS

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
        #if GL_ARB_buffer_storage
        case CPUAccess::ReadOnly:       return GL_MAP_READ_BIT;
        case CPUAccess::WriteOnly:      return GL_MAP_WRITE_BIT;
        case CPUAccess::WriteDiscard:   return GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
        case CPUAccess::ReadWrite:      return GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
        #endif
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

Texture* GLRenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const ImageView* initialImage)
{
    CreateGLContextOnce();
    ValidateGLTextureType(textureDesc.type);

    /* Create <GLTexture> object; will result in a GL renderbuffer or texture instance */
    auto* textureGL = textures_.emplace<GLTexture>(textureDesc);

    /* Initialize either renderbuffer or texture image storage */
    textureGL->BindAndAllocStorage(textureDesc, initialImage);

    return textureGL;
}

void GLRenderSystem::Release(Texture& texture)
{
    textures_.erase(&texture);
}

void GLRenderSystem::WriteTexture(Texture& texture, const TextureRegion& textureRegion, const ImageView& srcImageView)
{
    /* Bind texture and write texture sub data */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    textureGL.TextureSubImage(textureRegion, srcImageView, false);
}

void GLRenderSystem::ReadTexture(Texture& texture, const TextureRegion& textureRegion, const MutableImageView& dstImageView)
{
    /* Bind texture and write texture sub data */
    LLGL_ASSERT_PTR(dstImageView.data);
    auto& textureGL = LLGL_CAST(GLTexture&, texture);

    #if LLGL_GLEXT_MEMORY_BARRIERS
    if ((textureGL.GetBindFlags() & BindFlags::Storage) != 0)
    {
        /* Ensure all shader writes to the texture completed */
        if (HasExtension(GLExt::ARB_shader_image_load_store))
            glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
    }
    #endif // /LLGL_GLEXT_MEMORY_BARRIERS

    textureGL.GetTextureSubImage(textureRegion, dstImageView, false);
}

/* ----- Sampler States ---- */

Sampler* GLRenderSystem::CreateSampler(const SamplerDescriptor& samplerDesc)
{
    CreateGLContextOnce();
    if (!HasNativeSamplers())
    {
        /* If GL_ARB_sampler_objects is not supported, use emulated sampler states */
        auto* emulatedSamplerGL = emulatedSamplers_.emplace<GLEmulatedSampler>();
        emulatedSamplerGL->SamplerParameters(samplerDesc);
        return emulatedSamplerGL;
    }
    else
    {
        /* Create native GL sampler state */
        auto* samplerGL = samplers_.emplace<GLSampler>(samplerDesc.debugName);
        samplerGL->SamplerParameters(samplerDesc);
        return samplerGL;
    }
}

void GLRenderSystem::Release(Sampler& sampler)
{
    /* If GL_ARB_sampler_objects is not supported, release emulated sampler states */
    if (!HasNativeSamplers())
        emulatedSamplers_.erase(&sampler);
    else
        samplers_.erase(&sampler);
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
    /* Make sure we have a GLContext with compatible resolution */
    CreateGLContextOnce();
    LLGL_ASSERT_RENDERING_FEATURE_SUPPORT(hasRenderTargets);
    return renderTargets_.emplace<GLRenderTarget>(GetRenderingCaps().limits, renderTargetDesc);
}

void GLRenderSystem::Release(RenderTarget& renderTarget)
{
    renderTargets_.erase(&renderTarget);
}

/* ----- Shader ----- */

Shader* GLRenderSystem::CreateShader(const ShaderDescriptor& shaderDesc)
{
    CreateGLContextOnce();
    RenderSystem::AssertCreateShader(shaderDesc);

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

/* ----- Pipeline Caches ----- */

PipelineCache* GLRenderSystem::CreatePipelineCache(const Blob& initialBlob)
{
    if (GetRenderingCaps().features.hasPipelineCaching)
        return pipelineCaches_.emplace<GLPipelineCache>(initialBlob);
    else
        return ProxyPipelineCache::CreateInstance(pipelineCacheProxy_);
}

void GLRenderSystem::Release(PipelineCache& pipelineCache)
{
    if (GetRenderingCaps().features.hasPipelineCaching)
        pipelineCaches_.erase(&pipelineCache);
    else
        ProxyPipelineCache::ReleaseInstance(pipelineCacheProxy_, pipelineCache);
}

/* ----- Pipeline States ----- */

PipelineState* GLRenderSystem::CreatePipelineState(const GraphicsPipelineDescriptor& pipelineStateDesc, PipelineCache* pipelineCache)
{
    return pipelineStates_.emplace<GLGraphicsPSO>(
        pipelineStateDesc,
        GetRenderingCaps().limits,
        (GetRenderingCaps().features.hasPipelineCaching ? pipelineCache : nullptr)
    );
}

PipelineState* GLRenderSystem::CreatePipelineState(const ComputePipelineDescriptor& pipelineStateDesc, PipelineCache* pipelineCache)
{
    return pipelineStates_.emplace<GLComputePSO>(
        pipelineStateDesc,
        (GetRenderingCaps().features.hasPipelineCaching ? pipelineCache : nullptr)
    );
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

/* ----- Extensions ----- */

bool GLRenderSystem::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize != 0)
        return contextMngr_.AllocContext()->GetNativeHandle(nativeHandle, nativeHandleSize);
    else
        return false;
}


/*
 * ======= Private: =======
 */

void GLRenderSystem::CreateGLContextOnce()
{
    (void)contextMngr_.AllocContext();
}

void GLRenderSystem::RegisterNewGLContext(GLContext& /*context*/, const GLPixelFormat& pixelFormat)
{
    /* Enable debug callback function */
    if (debugContext_)
        EnableDebugCallback();
}

#if LLGL_GLEXT_DEBUG

#ifdef LLGL_OPENGL
void APIENTRY GLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* /*userParam*/)
#else
void GL_APIENTRY GLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* /*userParam*/)
#endif
{
    /* Forward callback to log */
    DebugPrintf(
        "glDebugMessageCallback (%s, %s, %s): %s",
        GLDebugSourceToStr(source), GLDebugTypeToStr(type), GLDebugSeverityToStr(severity), message
    );
}

#endif // /LLGL_GLEXT_DEBUG

struct GLDebugMessageMetaData
{
    GLenum source, type, severity;
};

void GLRenderSystem::EnableDebugCallback(bool enable)
{
    #if LLGL_GLEXT_DEBUG

    if (HasExtension(GLExt::KHR_debug))
    {
        if (enable)
        {
            /* Enable GL debug message callback */
            GLStateManager::Get().Enable(GLState::DebugOutput);
            GLStateManager::Get().Enable(GLState::DebugOutputSynchronous);
            glDebugMessageCallback(GLDebugCallback, nullptr);

            /* Filter out spam from debug callback */
            static constexpr GLDebugMessageMetaData debugMessageMetaData[] =
            {
                { GL_DEBUG_SOURCE_API,         GL_DEBUG_TYPE_OTHER, GL_DEBUG_SEVERITY_NOTIFICATION },
                { GL_DEBUG_SOURCE_APPLICATION, GL_DONT_CARE,        GL_DONT_CARE                   },
            };

            for (const auto& metaData : debugMessageMetaData)
                glDebugMessageControl(metaData.source, metaData.type, metaData.severity, 0, nullptr, GL_FALSE);
        }
        else
        {
            GLStateManager::Get().Disable(GLState::DebugOutput);
            GLStateManager::Get().Disable(GLState::DebugOutputSynchronous);
            glDebugMessageCallback(nullptr, nullptr);
        }
    }

    #endif // /LLGL_GLEXT_DEBUG
}

static std::string GLGetString(GLenum name)
{
    const GLubyte* bytes = glGetString(name);
    return (bytes != nullptr ? std::string(reinterpret_cast<const char*>(bytes)) : "");
}

static void GLQueryRendererInfo(RendererInfo& info)
{
    info.rendererName           = GLProfile::GetAPIName() + std::string(" ") + GLGetString(GL_VERSION);
    info.deviceName             = GLGetString(GL_RENDERER);
    info.vendorName             = GLGetString(GL_VENDOR);
    info.shadingLanguageName    = GLProfile::GetShadingLanguageName() + std::string(" ") + GLGetString(GL_SHADING_LANGUAGE_VERSION);

    const std::set<const char*>& extensionNames = GetLoadedOpenGLExtensions();
    info.extensionNames = std::vector<std::string>(extensionNames.begin(), extensionNames.end());

    GLQueryPipelineCacheID(info.pipelineCacheID);
}

static void AppendCacheIDBytes(std::vector<char>& cacheID, const void* bytes, std::size_t count)
{
    const std::size_t offset = cacheID.size();
    cacheID.resize(offset + count);
    ::memcpy(&cacheID[offset], bytes, count);
}

template <typename T>
static void AppendCacheIDValue(std::vector<char>& cacheID, const T& val)
{
    AppendCacheIDBytes(cacheID, &val, sizeof(val));
}

// Must not be static to be available in GL module
void GLQueryPipelineCacheID(std::vector<char>& cacheID)
{
    #ifdef GL_ARB_get_program_binary
    if (HasExtension(GLExt::ARB_get_program_binary))
    {
        GLint numBinaryFormats = 0;
        glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &numBinaryFormats);
        if (numBinaryFormats > 0)
        {
            /* Append number of binary formats */
            AppendCacheIDValue(cacheID, numBinaryFormats);

            /* Append binary format values themselves */
            std::vector<GLint> formats;
            formats.resize(numBinaryFormats);
            glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, formats.data());
            AppendCacheIDBytes(cacheID, formats.data(), sizeof(GLint) * formats.size());

            /* Append GL version string */
            if (const GLubyte* versionStr = glGetString(GL_VERSION))
                AppendCacheIDBytes(cacheID, versionStr, ::strlen(reinterpret_cast<const char*>(versionStr)));
        }
    }
    #endif // /GL_ARB_get_program_binary
}

bool GLRenderSystem::QueryRendererDetails(RendererInfo* outInfo, RenderingCapabilities* outCaps)
{
    if (outInfo != nullptr || outCaps != nullptr)
    {
        /* Make sure we have a GL context before querying information from it */
        CreateGLContextOnce();
        if (outInfo != nullptr)
            GLQueryRendererInfo(*outInfo);
        if (outCaps != nullptr)
            GLQueryRenderingCaps(*outCaps);
    }
    return true;
}


} // /namespace LLGL



// ================================================================================
