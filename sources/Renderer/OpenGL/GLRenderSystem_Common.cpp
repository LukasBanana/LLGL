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

std::map<RendererInfo, std::string> GLRenderSystem::QueryRendererInfo() const
{
    std::map<RendererInfo, std::string> info;

    std::vector<std::pair<RendererInfo, GLenum>> entries
    {{
        { RendererInfo::Version,                GL_VERSION                  },
        { RendererInfo::Vendor,                 GL_VENDOR                   },
        { RendererInfo::Hardware,               GL_RENDERER                 },
        { RendererInfo::ShadingLanguageVersion, GL_SHADING_LANGUAGE_VERSION },
    }};
    
    for (const auto& entry : entries)
    {
        auto bytes = glGetString(entry.second);
        if (bytes)
            info[entry.first] = std::string(reinterpret_cast<const char*>(bytes));
    }

    return info;
}

RenderingCaps GLRenderSystem::QueryRenderingCaps() const
{
    return GetRenderingCaps();
}

ShadingLanguage GLRenderSystem::QueryShadingLanguage() const
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

/* ----- Render Context ----- */

RenderContext* GLRenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window)
{
    return AddRenderContext(MakeUnique<GLRenderContext>(*this, desc, window, nullptr), desc, window);
}

void GLRenderSystem::Release(RenderContext& renderContext)
{
    if (GetCurrentContext() == &renderContext)
        MakeCurrent(nullptr);
    RemoveFromUniqueSet(renderContexts_, &renderContext);
}

/* ----- Hardware Buffers ------ */

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

void GLRenderSystem::Release(Sampler& sampler)
{
    RemoveFromUniqueSet(samplers_, &sampler);
}

/* ----- Render Targets ----- */

RenderTarget* GLRenderSystem::CreateRenderTarget(unsigned int multiSamples)
{
    LLGL_ASSERT_CAP(hasRenderTargets);
    return TakeOwnership(renderTargets_, MakeUnique<GLRenderTarget>(multiSamples));
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

Query* GLRenderSystem::CreateQuery(const QueryType type)
{
    return TakeOwnership(queries_, MakeUnique<GLQuery>(type));
}

void GLRenderSystem::Release(Query& query)
{
    RemoveFromUniqueSet(queries_, &query);
}

/* ----- Extended Internal Functions ----- */

bool GLRenderSystem::HasExtension(const std::string& name) const
{
    return (extensionMap_.find(name) != extensionMap_.end());
}


/*
 * ======= Protected: =======
 */

RenderContext* GLRenderSystem::AddRenderContext(
    std::unique_ptr<GLRenderContext>&& renderContext, const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window)
{
    /*
    If render context created it's own window then show it after creation,
    since anti-aliasing may force the window to be recreated several times
    */
    if (!window)
        renderContext->GetWindow().Show();

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
    GLStateManager::active->SetClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE);

    /* Take ownership and return raw pointer */
    return TakeOwnership(renderContexts_, std::move(renderContext));
}


/*
 * ======= Private: =======
 */

bool GLRenderSystem::OnMakeCurrent(RenderContext* renderContext)
{
    if (renderContext)
    {
        auto renderContextGL = LLGL_CAST(GLRenderContext*, renderContext);
        return GLRenderContext::GLMakeCurrent(renderContextGL);
    }
    else
        return GLRenderContext::GLMakeCurrent(nullptr);
}

void GLRenderSystem::LoadGLExtensions(const ProfileOpenGLDescriptor& profileDesc)
{
    /* Load OpenGL extensions if not already done */
    if (extensionMap_.empty())
    {
        auto coreProfile = (profileDesc.extProfile && profileDesc.coreProfile);

        /* Query extensions and load all of them */
        extensionMap_ = QueryExtensions(coreProfile);
        LoadAllExtensions(extensionMap_);

        /* Query and store all rendering capabilities */
        StoreRenderingCaps();
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

void GLRenderSystem::BindTextureAndSetType(GLTexture& textureGL, const TextureType type)
{
    if (textureGL.GetType() == type)
    {
        /* If type is already set, just bind texture */
        GLStateManager::active->BindTexture(textureGL);
    }
    else
    {
        /* If texture is not undefined -> recreate it */
        if (textureGL.GetType() != TextureType::Undefined)
            textureGL.Recreate();

        /* Set type of the specified texture for the first time */
        textureGL.SetType(type);
        GLStateManager::active->ForcedBindTexture(textureGL);

        /* Setup texture parameters for the first time */
        auto target = GLTypes::Map(type);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
}

void GLRenderSystem::StoreRenderingCaps()
{
    auto& caps = renderingCaps_;

    /* Set fixed states for this renderer */
    caps.screenOrigin                   = ScreenOrigin::LowerLeft;
    caps.clippingRange                  = ClippingRange::MinusOneToOne;
    caps.hasGLSL                        = true;

    /* Query all boolean capabilies by their respective OpenGL extension */
    caps.hasRenderTargets               = HasExtension("GL_ARB_framebuffer_object");
    caps.has3DTextures                  = HasExtension("GL_EXT_texture3D");
    caps.hasCubeTextures                = HasExtension("GL_ARB_texture_cube_map");
    caps.hasTextureArrays               = HasExtension("GL_EXT_texture_array");
    caps.hasCubeTextureArrays           = HasExtension("GL_ARB_texture_cube_map_array");
    caps.hasSamplers                    = HasExtension("GL_ARB_sampler_objects");
    caps.hasConstantBuffers             = HasExtension("GL_ARB_uniform_buffer_object");
    caps.hasStorageBuffers              = HasExtension("GL_ARB_shader_storage_buffer_object");
    caps.hasUniforms                    = HasExtension("GL_ARB_shader_objects");
    caps.hasGeometryShaders             = HasExtension("GL_ARB_geometry_shader4");
    caps.hasTessellationShaders         = HasExtension("GL_ARB_tessellation_shader");
    caps.hasComputeShaders              = HasExtension("GL_ARB_compute_shader");
    caps.hasInstancing                  = HasExtension("GL_ARB_draw_instanced");
    caps.hasOffsetInstancing            = HasExtension("GL_ARB_base_instance");
    caps.hasViewportArrays              = HasExtension("GL_ARB_viewport_array");
    caps.hasConservativeRasterization   = (HasExtension("GL_NV_conservative_raster") || HasExtension("GL_INTEL_conservative_rasterization"));

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
        if (glGetIntegeri_v)
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
        throw std::runtime_error(feature + " are not supported by the OpenGL renderer");
    }
}


} // /namespace LLGL



// ================================================================================
