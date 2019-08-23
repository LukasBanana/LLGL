/*
 * GLRenderSystem_Common.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderSystem.h"
#include "Ext/GLExtensions.h"
#include "RenderState/GLStatePool.h"
#include "../RenderSystemUtils.h"
#include "../GLCommon/GLTypes.h"
#include "../GLCommon/GLCore.h"
#include "../GLCommon/Texture/GLTexImage.h"
#include "../CheckedCast.h"
#include "../../Core/Helper.h"
#include "../../Core/Assertion.h"
#include "GLRenderingCaps.h"
#include "Command/GLImmediateCommandBuffer.h"
#include "Command/GLDeferredCommandBuffer.h"


namespace LLGL
{


/* ----- Common ----- */

GLRenderSystem::GLRenderSystem(const RenderSystemDescriptor& renderSystemDesc)
{
    /* Extract optional renderer configuartion */
    if (auto rendererConfigGL = GetRendererConfiguration<RendererConfigurationOpenGL>(renderSystemDesc))
        config_ = *rendererConfigGL;
}

GLRenderSystem::~GLRenderSystem()
{
    /* Clear all render state containers first, the rest will be deleted automatically */
    GLStatePool::Instance().Clear();
}

/* ----- Render Context ----- */

// private
GLRenderContext* GLRenderSystem::GetSharedRenderContext() const
{
    return (!renderContexts_.empty() ? renderContexts_.begin()->get() : nullptr);
}

RenderContext* GLRenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Surface>& surface)
{
    return AddRenderContext(MakeUnique<GLRenderContext>(desc, config_, surface, GetSharedRenderContext()));
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

CommandBuffer* GLRenderSystem::CreateCommandBuffer(const CommandBufferDescriptor& desc)
{
    /* Get state manager from shared render context */
    if (auto sharedContext = GetSharedRenderContext())
    {
        if ((desc.flags & (CommandBufferFlags::DeferredSubmit | CommandBufferFlags::MultiSubmit)) != 0)
        {
            /* Create deferred command buffer */
            return TakeOwnership(
                commandBuffers_,
                MakeUnique<GLDeferredCommandBuffer>(desc.flags)
            );
        }
        else
        {
            /* Create immediate command buffer */
            return TakeOwnership(
                commandBuffers_,
                MakeUnique<GLImmediateCommandBuffer>(sharedContext->GetStateManager())
            );
        }
    }
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

void GLRenderSystem::Release(Sampler& sampler)
{
    RemoveFromUniqueSet(samplers_, &sampler);
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

/* ----- Render Passes ----- */

RenderPass* GLRenderSystem::CreateRenderPass(const RenderPassDescriptor& desc)
{
    AssertCreateRenderPass(desc);
    return TakeOwnership(renderPasses_, MakeUnique<GLRenderPass>(desc));
}

void GLRenderSystem::Release(RenderPass& renderPass)
{
    RemoveFromUniqueSet(renderPasses_, &renderPass);
}

/* ----- Render Targets ----- */

RenderTarget* GLRenderSystem::CreateRenderTarget(const RenderTargetDescriptor& desc)
{
    LLGL_ASSERT_FEATURE_SUPPORT(hasRenderTargets);
    AssertCreateRenderTarget(desc);
    return TakeOwnership(renderTargets_, MakeUnique<GLRenderTarget>(desc));
}

void GLRenderSystem::Release(RenderTarget& renderTarget)
{
    RemoveFromUniqueSet(renderTargets_, &renderTarget);
}

/* ----- Shader ----- */

Shader* GLRenderSystem::CreateShader(const ShaderDescriptor& desc)
{
    AssertCreateShader(desc);

    /* Validate rendering capabilities for required shader type */
    switch (desc.type)
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
    return TakeOwnership(shaders_, MakeUnique<GLShader>(desc));
}

ShaderProgram* GLRenderSystem::CreateShaderProgram(const ShaderProgramDescriptor& desc)
{
    AssertCreateShaderProgram(desc);
    return TakeOwnership(shaderPrograms_, MakeUnique<GLShaderProgram>(desc));
}

void GLRenderSystem::Release(Shader& shader)
{
    RemoveFromUniqueSet(shaders_, &shader);
}

void GLRenderSystem::Release(ShaderProgram& shaderProgram)
{
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

QueryHeap* GLRenderSystem::CreateQueryHeap(const QueryHeapDescriptor& desc)
{
    return TakeOwnership(queryHeaps_, MakeUnique<GLQueryHeap>(desc));
}

void GLRenderSystem::Release(QueryHeap& queryHeap)
{
    RemoveFromUniqueSet(queryHeaps_, &queryHeap);
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

RenderContext* GLRenderSystem::AddRenderContext(std::unique_ptr<GLRenderContext>&& renderContext)
{
    /* Create devices that require an active GL context */
    if (renderContexts_.empty())
        CreateGLContextDependentDevices(*renderContext);

    /* Use uniform clipping space */
    GLStateManager::Get().DetermineExtensionsAndLimits();
    GLStateManager::Get().SetClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE);

    /* Take ownership and return raw pointer */
    return TakeOwnership(renderContexts_, std::move(renderContext));
}


/*
 * ======= Private: =======
 */

void GLRenderSystem::CreateGLContextDependentDevices(GLRenderContext& renderContext)
{
    const bool hasGLCoreProfile = (config_.contextProfile == OpenGLContextProfile::CoreProfile);

    /* Load all OpenGL extensions */
    LoadGLExtensions(hasGLCoreProfile);

    /* Enable debug callback function */
    if (debugCallback_)
        SetDebugCallback(debugCallback_);

    /* Create command queue instance */
    commandQueue_ = MakeUnique<GLCommandQueue>(renderContext.GetStateManager());
}

void GLRenderSystem::LoadGLExtensions(bool hasGLCoreProfile)
{
    /* Load OpenGL extensions if not already done */
    if (!AreExtensionsLoaded())
    {
        /* Query extensions and load all of them */
        auto extensions = QueryExtensions(hasGLCoreProfile);
        LoadAllExtensions(extensions, hasGLCoreProfile);

        /* Query and store all renderer information and capabilities */
        QueryRendererInfo();
        QueryRenderingCaps();
    }
}

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
    auto debugCallback = reinterpret_cast<const DebugCallback*>(userParam);
    (*debugCallback)(typeStr.str(), message);
}

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


#ifdef LLGL_ENABLE_CUSTOM_SUB_MIPGEN

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

#endif // /LLGL_ENABLE_CUSTOM_SUB_MIPGEN


} // /namespace LLGL



// ================================================================================
