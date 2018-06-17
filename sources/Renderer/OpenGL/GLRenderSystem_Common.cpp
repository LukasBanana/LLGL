/*
 * GLRenderSystem_Common.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderSystem.h"
#include "../GLCommon/GLTypes.h"
#include "../GLCommon/GLCore.h"
#include "../GLCommon/Texture/GLTexImage.h"
#include "Ext/GLExtensions.h"
#include "../CheckedCast.h"
#include "../../Core/Helper.h"
#include "../../Core/Assertion.h"
#include "GLRenderingCaps.h"


namespace LLGL
{


/* ----- Render System ----- */

void GLRenderSystem::SetConfiguration(const RenderSystemConfiguration& config)
{
    RenderSystem::SetConfiguration(config);
    GLTexImageInitialization(config.imageInitialization);
}

/* ----- Render Context ----- */

// private
GLRenderContext* GLRenderSystem::GetSharedRenderContext() const
{
    return (!renderContexts_.empty() ? renderContexts_.begin()->get() : nullptr);
}

RenderContext* GLRenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Surface>& surface)
{
    return AddRenderContext(MakeUnique<GLRenderContext>(desc, surface, GetSharedRenderContext()), desc);
}

void GLRenderSystem::Release(RenderContext& renderContext)
{
    RemoveFromUniqueSet(renderContexts_, &renderContext);
}

/* ----- Command queues ----- */

CommandQueue* GLRenderSystem::GetCommandQueue()
{
    return commandQueue_.get();
}

/* ----- Command buffers ----- */

CommandBuffer* GLRenderSystem::CreateCommandBuffer()
{
    return CreateCommandBufferExt();
}

CommandBufferExt* GLRenderSystem::CreateCommandBufferExt()
{
    /* Get state manager from shared render context */
    if (auto sharedContext = GetSharedRenderContext())
        return TakeOwnership(commandBuffers_, MakeUnique<GLCommandBuffer>(sharedContext->GetStateManager()));
    else
        throw std::runtime_error("cannot create OpenGL command buffer without active render context");
}

void GLRenderSystem::Release(CommandBuffer& commandBuffer)
{
    RemoveFromUniqueSet(commandBuffers_, &commandBuffer);
}

/* ----- Buffers ------ */

// --> see "GLRenderSystem_Buffers.cpp" file

/* ----- Textures ----- */

// --> see "GLRenderSystem_Textures.cpp" file

/* ----- Sampler States ---- */

Sampler* GLRenderSystem::CreateSampler(const SamplerDescriptor& desc)
{
    LLGL_ASSERT_FEATURE_SUPPORT(hasSamplers);
    auto sampler = MakeUnique<GLSampler>();
    sampler->SetDesc(desc);
    return TakeOwnership(samplers_, std::move(sampler));
}

SamplerArray* GLRenderSystem::CreateSamplerArray(std::uint32_t numSamplers, Sampler* const * samplerArray)
{
    LLGL_ASSERT_FEATURE_SUPPORT(hasSamplers);
    AssertCreateSamplerArray(numSamplers, samplerArray);
    return TakeOwnership(samplerArrays_, MakeUnique<GLSamplerArray>(numSamplers, samplerArray));
}

void GLRenderSystem::Release(Sampler& sampler)
{
    auto& samplerGL = LLGL_CAST(GLSampler&, sampler);
    RemoveFromUniqueSet(samplers_, &sampler);
}

void GLRenderSystem::Release(SamplerArray& samplerArray)
{
    auto& samplerArrayGL = LLGL_CAST(GLSamplerArray&, samplerArray);
    RemoveFromUniqueSet(samplerArrays_, &samplerArray);
}

/* ----- Resource Heaps ----- */

ResourceHeap* GLRenderSystem::CreateResourceHeap(const ResourceHeapDescriptor& desc)
{
    return TakeOwnership(resourceHeaps_, MakeUnique<GLResourceHeap>(desc));
}

void GLRenderSystem::Release(ResourceHeap& resourceHeap)
{
    RemoveFromUniqueSet(resourceHeaps_, &resourceHeap);
}

/* ----- Render Targets ----- */

RenderTarget* GLRenderSystem::CreateRenderTarget(const RenderTargetDescriptor& desc)
{
    LLGL_ASSERT_FEATURE_SUPPORT(hasRenderTargets);
    return TakeOwnership(renderTargets_, MakeUnique<GLRenderTarget>(desc));
}

void GLRenderSystem::Release(RenderTarget& renderTarget)
{
    /* Release render target (GLRenderTarget destructor notifies GL state manager about object releases) */
    auto& renderTargetGL = LLGL_CAST(GLRenderTarget&, renderTarget);
    RemoveFromUniqueSet(renderTargets_, &renderTarget);
}

/* ----- Shader ----- */

Shader* GLRenderSystem::CreateShader(const ShaderType type)
{
    /* Validate rendering capabilities for required shader type */
    switch (type)
    {
        case ShaderType::Geometry:
            LLGL_ASSERT_FEATURE_SUPPORT(hasGeometryShaders);
            break;
        case ShaderType::TessControl:
        case ShaderType::TessEvaluation:
            LLGL_ASSERT_FEATURE_SUPPORT(hasTessellationShaders);
            break;
        case ShaderType::Compute:
            LLGL_ASSERT_FEATURE_SUPPORT(hasComputeShaders);
            break;
        default:
            break;
    }

    /* Make and return shader object */
    return TakeOwnership(shaders_, MakeUnique<GLShader>(type));
}

ShaderProgram* GLRenderSystem::CreateShaderProgram()
{
    return TakeOwnership(shaderPrograms_, MakeUnique<GLShaderProgram>());
}

void GLRenderSystem::Release(Shader& shader)
{
    RemoveFromUniqueSet(shaders_, &shader);
}

void GLRenderSystem::Release(ShaderProgram& shaderProgram)
{
    auto& shaderProgramGL = LLGL_CAST(GLShaderProgram&, shaderProgram);
    RemoveFromUniqueSet(shaderPrograms_, &shaderProgram);
}

/* ----- Pipeline Layouts ----- */

PipelineLayout* GLRenderSystem::CreatePipelineLayout(const PipelineLayoutDescriptor& desc)
{
    return TakeOwnership(pipelineLayouts_, MakeUnique<GLPipelineLayout>(desc));
}

void GLRenderSystem::Release(PipelineLayout& pipelineLayout)
{
    RemoveFromUniqueSet(pipelineLayouts_, &pipelineLayout);
}

/* ----- Pipeline States ----- */

GraphicsPipeline* GLRenderSystem::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc)
{
    return TakeOwnership(graphicsPipelines_, MakeUnique<GLGraphicsPipeline>(desc, GetRenderingCaps().limits));
}

ComputePipeline* GLRenderSystem::CreateComputePipeline(const ComputePipelineDescriptor& desc)
{
    return TakeOwnership(computePipelines_, MakeUnique<GLComputePipeline>(desc));
}

void GLRenderSystem::Release(GraphicsPipeline& graphicsPipeline)
{
    RemoveFromUniqueSet(graphicsPipelines_, &graphicsPipeline);
}

void GLRenderSystem::Release(ComputePipeline& computePipeline)
{
    RemoveFromUniqueSet(computePipelines_, &computePipeline);
}

/* ----- Queries ----- */

Query* GLRenderSystem::CreateQuery(const QueryDescriptor& desc)
{
    return TakeOwnership(queries_, MakeUnique<GLQuery>(desc));
}

void GLRenderSystem::Release(Query& query)
{
    RemoveFromUniqueSet(queries_, &query);
}

/* ----- Fences ----- */

Fence* GLRenderSystem::CreateFence()
{
    return TakeOwnership(fences_, MakeUnique<GLFence>());
}

void GLRenderSystem::Release(Fence& fence)
{
    RemoveFromUniqueSet(fences_, &fence);
}


/*
 * ======= Protected: =======
 */

RenderContext* GLRenderSystem::AddRenderContext(std::unique_ptr<GLRenderContext>&& renderContext, const RenderContextDescriptor& desc)
{
    /* Load all OpenGL extensions for the first time */
    if (renderContexts_.empty())
    {
        LoadGLExtensions(desc.profileOpenGL);
        SetDebugCallback(desc.debugCallback);
        commandQueue_ = MakeUnique<GLCommandQueue>();
    }

    /* Use uniform clipping space */
    GLStateManager::active->DetermineExtensionsAndLimits();
    GLStateManager::active->SetClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE);

    /* Take ownership and return raw pointer */
    return TakeOwnership(renderContexts_, std::move(renderContext));
}


/*
 * ======= Private: =======
 */

void GLRenderSystem::LoadGLExtensions(const ProfileOpenGLDescriptor& profileDesc)
{
    /* Load OpenGL extensions if not already done */
    if (!AreExtensionsLoaded())
    {
        auto coreProfile = (profileDesc.contextProfile == OpenGLContextProfile::CoreProfile);

        /* Query extensions and load all of them */
        auto extensions = QueryExtensions(coreProfile);
        LoadAllExtensions(extensions, coreProfile);

        /* Query and store all renderer information and capabilities */
        QueryRendererInfo();
        QueryRenderingCaps();
    }
}

#ifdef LLGL_DEBUG

void APIENTRY GLDebugCallback(
    GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    /* Generate output stream */
    std::stringstream typeStr;

    typeStr
        << "OpenGL debug callback ("
        << GLDebugSourceToStr(source) << ", "
        << GLDebugTypeToStr(type) << ", "
        << GLDebugSeverityToStr(severity) << ")";

    /* Call debug callback */
    auto& debugCallback = *reinterpret_cast<const DebugCallback*>(userParam);
    debugCallback(typeStr.str(), message);
}

#endif

void GLRenderSystem::SetDebugCallback(const DebugCallback& debugCallback)
{
    #if defined(LLGL_DEBUG) && !defined(__APPLE__)

    debugCallback_ = debugCallback;

    if (debugCallback_)
    {
        GLStateManager::active->Enable(GLState::DEBUG_OUTPUT);
        GLStateManager::active->Enable(GLState::DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(GLDebugCallback, &debugCallback_);
    }
    else
    {
        GLStateManager::active->Disable(GLState::DEBUG_OUTPUT);
        GLStateManager::active->Disable(GLState::DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(nullptr, nullptr);
    }

    #endif
}

static std::string GLGetString(GLenum name)
{
    auto bytes = glGetString(name);
    return (bytes != nullptr ? std::string(reinterpret_cast<const char*>(bytes)) : "");
}

void GLRenderSystem::QueryRendererInfo()
{
    RendererInfo info;

    info.rendererName           = "OpenGL " + GLGetString(GL_VERSION);
    info.deviceName             = GLGetString(GL_RENDERER);
    info.vendorName             = GLGetString(GL_VENDOR);
    info.shadingLanguageName    = "GLSL " + GLGetString(GL_SHADING_LANGUAGE_VERSION);

    SetRendererInfo(info);
}

void GLRenderSystem::QueryRenderingCaps()
{
    RenderingCapabilities caps;
    GLQueryRenderingCaps(caps);
    SetRenderingCaps(caps);
}


/*
 * MipGenerationFBOPair structure
 */

GLRenderSystem::MipGenerationFBOPair::~MipGenerationFBOPair()
{
    ReleaseFBOs();
}

void GLRenderSystem::MipGenerationFBOPair::CreateFBOs()
{
    if (!fbos[0])
        glGenFramebuffers(2, fbos);
}

void GLRenderSystem::MipGenerationFBOPair::ReleaseFBOs()
{
    if (fbos[0])
    {
        glDeleteFramebuffers(2, fbos);
        fbos[0] = 0;
        fbos[1] = 0;
    }
}


} // /namespace LLGL



// ================================================================================
