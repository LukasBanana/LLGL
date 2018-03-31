/*
 * GLRenderSystem_Common.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderSystem.h"
#include "../GLCommon/GLTypes.h"
#include "../GLCommon/GLCore.h"
#include "../GLCommon/Texture/GLTexImage.h"
#include "Ext/GLExtensions.h"
#include "../CheckedCast.h"
#include "../../Core/Helper.h"
#include "../../Core/Exception.h"
#include "GLRenderingCaps.h"
#include <LLGL/Desktop.h>

#include <LLGL/Log.h>


namespace LLGL
{


/* ----- Render System ----- */

GLRenderSystem::GLRenderSystem()
{
}

GLRenderSystem::~GLRenderSystem()
{
    Desktop::ResetVideoMode();
}

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
    /* Get state manager from shared render context */
    auto sharedContext = GetSharedRenderContext();
    if (!sharedContext)
        throw std::runtime_error("cannot create OpenGL command buffer without active render context");

    /* Create command buffer */
    return TakeOwnership(commandBuffers_, MakeUnique<GLCommandBuffer>(sharedContext->GetStateManager()));
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
    LLGL_ASSERT_CAP(hasSamplers);
    auto sampler = MakeUnique<GLSampler>();
    sampler->SetDesc(desc);
    return TakeOwnership(samplers_, std::move(sampler));
}

SamplerArray* GLRenderSystem::CreateSamplerArray(std::uint32_t numSamplers, Sampler* const * samplerArray)
{
    LLGL_ASSERT_CAP(hasSamplers);
    AssertCreateSamplerArray(numSamplers, samplerArray);
    return TakeOwnership(samplerArrays_, MakeUnique<GLSamplerArray>(numSamplers, samplerArray));
}

void GLRenderSystem::Release(Sampler& sampler)
{
    RemoveFromUniqueSet(samplers_, &sampler);
}

void GLRenderSystem::Release(SamplerArray& samplerArray)
{
    RemoveFromUniqueSet(samplerArrays_, &samplerArray);
}

/* ----- Render Targets ----- */

RenderTarget* GLRenderSystem::CreateRenderTarget(const RenderTargetDescriptor& desc)
{
    LLGL_ASSERT_CAP(hasRenderTargets);
    return TakeOwnership(renderTargets_, MakeUnique<GLRenderTarget>(desc));
}

void GLRenderSystem::Release(RenderTarget& renderTarget)
{
    RemoveFromUniqueSet(renderTargets_, &renderTarget);
}

/* ----- Shader ----- */

Shader* GLRenderSystem::CreateShader(const ShaderType type)
{
    /* Validate rendering capabilities for required shader type */
    switch (type)
    {
        case ShaderType::Geometry:
            LLGL_ASSERT_CAP(hasGeometryShaders);
            break;
        case ShaderType::TessControl:
        case ShaderType::TessEvaluation:
            LLGL_ASSERT_CAP(hasTessellationShaders);
            break;
        case ShaderType::Compute:
            LLGL_ASSERT_CAP(hasComputeShaders);
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
    RemoveFromUniqueSet(shaderPrograms_, &shaderProgram);
}

/* ----- Pipeline States ----- */

GraphicsPipeline* GLRenderSystem::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc)
{
    return TakeOwnership(graphicsPipelines_, MakeUnique<GLGraphicsPipeline>(desc, GetRenderingCaps()));
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
    /* Switch to fullscreen mode (if enabled) */
    if (desc.videoMode.fullscreen)
        Desktop::SetVideoMode(desc.videoMode);

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
        auto coreProfile = (profileDesc.extProfile && profileDesc.coreProfile);

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
    RenderingCaps caps;
    GLQueryRenderingCaps(caps);
    SetRenderingCaps(caps);
}

void GLRenderSystem::AssertCap(bool supported, const std::string& memberName)
{
    if (!supported)
    {
        /* Remove "has" from name */
        auto feature = memberName;
        if (feature.size() > 3 && feature.substr(0, 3) == "has")
            feature = feature.substr(3);

        /* Throw descriptive error */
        ThrowNotSupported(feature);
    }
}


} // /namespace LLGL



// ================================================================================
