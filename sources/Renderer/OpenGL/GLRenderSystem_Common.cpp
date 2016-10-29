/*
 * GLRenderSystem_Common.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderSystem.h"
#include "GLTypes.h"
#include "GLCore.h"
#include "Ext/GLExtensions.h"
#include "../CheckedCast.h"
#include "../../Core/Helper.h"
#include "../../Core/Exception.h"
#include <LLGL/Desktop.h>


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

/* ----- Render Context ----- */

// private
GLRenderContext* GLRenderSystem::GetSharedRenderContext() const
{
    return (!renderContexts_.empty() ? renderContexts_.begin()->get() : nullptr);
}

RenderContext* GLRenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window)
{
    return AddRenderContext(MakeUnique<GLRenderContext>(desc, window, GetSharedRenderContext()), desc, window);
}

void GLRenderSystem::Release(RenderContext& renderContext)
{
    RemoveFromUniqueSet(renderContexts_, &renderContext);
}

/* ----- Command buffers ----- */

CommandBuffer* GLRenderSystem::CreateCommandBuffer()
{
    /* Get state manager from shared render context */
    auto sharedContext = GetSharedRenderContext();
    if (!sharedContext)
        throw std::runtime_error("can not create OpenGL command buffer without active render context");

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

SamplerArray* GLRenderSystem::CreateSamplerArray(unsigned int numSamplers, Sampler* const * samplerArray)
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


/*
 * ======= Protected: =======
 */

RenderContext* GLRenderSystem::AddRenderContext(
    std::unique_ptr<GLRenderContext>&& renderContext, const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window)
{
    /* Switch to fullscreen mode (if enabled) */
    if (desc.videoMode.fullscreen)
        Desktop::SetVideoMode(desc.videoMode);

    /* Load all OpenGL extensions for the first time */
    if (renderContexts_.empty())
    {
        LoadGLExtensions(desc.profileOpenGL);
        SetDebugCallback(desc.debugCallback);
    }

    /* Use uniform clipping space */
    GLStateManager::active->DetermineExtensions();
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
        LoadAllExtensions(extensions);

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

    if (debugCallback)
    {
        GLStateManager::active->Enable(GLState::DEBUG_OUTPUT);
        GLStateManager::active->Enable(GLState::DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(GLDebugCallback, &debugCallback);
    }
    else
    {
        GLStateManager::active->Disable(GLState::DEBUG_OUTPUT);
        GLStateManager::active->Disable(GLState::DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(nullptr, nullptr);
    }

    #endif
}

static ShadingLanguage QueryShadingLanguage()
{
    /* Derive shading language version by OpenGL version */
    GLint major = 0, minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    auto IsVer = [major, minor](int maj, int min)
    {
        return (major == maj && minor == min);
    };

    if (IsVer(2, 0)) return ShadingLanguage::GLSL_110;
    if (IsVer(2, 1)) return ShadingLanguage::GLSL_120;
    if (IsVer(3, 0)) return ShadingLanguage::GLSL_130;
    if (IsVer(3, 1)) return ShadingLanguage::GLSL_140;
    if (IsVer(3, 2)) return ShadingLanguage::GLSL_150;
    if (IsVer(3, 3)) return ShadingLanguage::GLSL_330;
    if (IsVer(4, 0)) return ShadingLanguage::GLSL_400;
    if (IsVer(4, 1)) return ShadingLanguage::GLSL_410;
    if (IsVer(4, 2)) return ShadingLanguage::GLSL_420;
    if (IsVer(4, 3)) return ShadingLanguage::GLSL_430;
    if (IsVer(4, 4)) return ShadingLanguage::GLSL_440;
    if (IsVer(4, 5)) return ShadingLanguage::GLSL_450;

    return ShadingLanguage::GLSL_110;
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

    /* Set fixed states for this renderer */
    caps.screenOrigin                   = ScreenOrigin::LowerLeft;
    caps.clippingRange                  = ClippingRange::MinusOneToOne;
    caps.shadingLanguage                = QueryShadingLanguage();

    /* Query all boolean capabilies by their respective OpenGL extension */
    caps.hasRenderTargets               = HasExtension(GLExt::ARB_framebuffer_object);
    caps.has3DTextures                  = HasExtension(GLExt::EXT_texture3D);
    caps.hasCubeTextures                = HasExtension(GLExt::ARB_texture_cube_map);
    caps.hasTextureArrays               = HasExtension(GLExt::EXT_texture_array);
    caps.hasCubeTextureArrays           = HasExtension(GLExt::ARB_texture_cube_map_array);
    caps.hasMultiSampleTextures         = HasExtension(GLExt::ARB_texture_multisample);
    caps.hasSamplers                    = HasExtension(GLExt::ARB_sampler_objects);
    caps.hasConstantBuffers             = HasExtension(GLExt::ARB_uniform_buffer_object);
    caps.hasStorageBuffers              = HasExtension(GLExt::ARB_shader_storage_buffer_object);
    caps.hasUniforms                    = HasExtension(GLExt::ARB_shader_objects);
    caps.hasGeometryShaders             = HasExtension(GLExt::ARB_geometry_shader4);
    caps.hasTessellationShaders         = HasExtension(GLExt::ARB_tessellation_shader);
    caps.hasComputeShaders              = HasExtension(GLExt::ARB_compute_shader);
    caps.hasInstancing                  = HasExtension(GLExt::ARB_draw_instanced);
    caps.hasOffsetInstancing            = HasExtension(GLExt::ARB_base_instance);
    caps.hasViewportArrays              = HasExtension(GLExt::ARB_viewport_array);
    caps.hasConservativeRasterization   = ( HasExtension(GLExt::NV_conservative_raster) || HasExtension(GLExt::INTEL_conservative_rasterization) );
    caps.hasStreamOutputs               = ( HasExtension(GLExt::EXT_transform_feedback) || HasExtension(GLExt::NV_transform_feedback) );

    /* Query integral attributes */
    auto GetInt = [](GLenum param)
    {
        GLint attr = 0;
        glGetIntegerv(param, &attr);
        return attr;
    };

    auto GetUInt = [&](GLenum param)
    {
        return static_cast<unsigned int>(GetInt(param));
    };

    auto GetUIntIdx = [](GLenum param, GLuint index)
    {
        GLint attr = 0;
        if (HasExtension(GLExt::EXT_draw_buffers2))
            glGetIntegeri_v(param, index, &attr);
        return static_cast<unsigned int>(attr);
    };

    caps.maxNumTextureArrayLayers           = GetUInt(GL_MAX_ARRAY_TEXTURE_LAYERS);
    caps.maxNumRenderTargetAttachments      = GetUInt(GL_MAX_DRAW_BUFFERS);
    caps.maxConstantBufferSize              = GetUInt(GL_MAX_UNIFORM_BLOCK_SIZE);
    caps.maxPatchVertices                   = GetInt(GL_MAX_PATCH_VERTICES);
    caps.maxAnisotropy                      = 16;
    
    caps.maxNumComputeShaderWorkGroups.x    = GetUIntIdx(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0);
    caps.maxNumComputeShaderWorkGroups.y    = GetUIntIdx(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1);
    caps.maxNumComputeShaderWorkGroups.z    = GetUIntIdx(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2);

    caps.maxComputeShaderWorkGroupSize.x    = GetUIntIdx(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0);
    caps.maxComputeShaderWorkGroupSize.y    = GetUIntIdx(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1);
    caps.maxComputeShaderWorkGroupSize.z    = GetUIntIdx(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2);

    /* Query maximum texture dimensions */
    GLint querySizeBase = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &querySizeBase);

    /* Query 1D texture max size */
    auto querySize = querySizeBase;

    while (caps.max1DTextureSize == 0 && querySize > 0)
    {
        glTexImage1D(GL_PROXY_TEXTURE_1D, 0, GL_RGBA, querySize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_1D, 0, GL_TEXTURE_WIDTH, &(caps.max1DTextureSize));
        querySize /= 2;
    }

    /* Query 2D texture max size */
    querySize = querySizeBase;

    while (caps.max2DTextureSize == 0 && querySize > 0)
    {
        glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, querySize, querySize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &(caps.max2DTextureSize));
        querySize /= 2;
    }

    /* Query 3D texture max size */
    if (caps.has3DTextures)
    {
        querySize = querySizeBase;

        while (caps.max3DTextureSize == 0 && querySize > 0)
        {
            glTexImage3D(GL_PROXY_TEXTURE_3D, 0, GL_RGBA, querySize, querySize, querySize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glGetTexLevelParameteriv(GL_PROXY_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &(caps.max3DTextureSize));
            querySize /= 2;
        }
    }

    /* Query cube texture max size */
    if (caps.hasCubeTextures)
    {
        querySize = querySizeBase;

        while (caps.maxCubeTextureSize == 0 && querySize > 0)
        {
            glTexImage2D(GL_PROXY_TEXTURE_CUBE_MAP, 0, GL_RGBA, querySize, querySize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glGetTexLevelParameteriv(GL_PROXY_TEXTURE_CUBE_MAP, 0, GL_TEXTURE_WIDTH, &(caps.maxCubeTextureSize));
            querySize /= 2;
        }
    }

    /* Finally store queried rendering capabilities */
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
