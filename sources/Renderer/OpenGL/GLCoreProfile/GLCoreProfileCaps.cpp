/*
 * GLCoreProfileCaps.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../GLRenderingCaps.h"
#include "../GLTypes.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../../../Core/Helper.h"
#include <cstdint>
#include <limits>


namespace LLGL
{


static std::int32_t GLGetInt(GLenum param)
{
    GLint attr = 0;
    glGetIntegerv(param, &attr);
    return attr;
}

static std::uint32_t GLGetUInt(GLenum param)
{
    return static_cast<std::uint32_t>(GLGetInt(param));
};

static std::uint32_t GLGetUIntIndexed(GLenum param, GLuint index)
{
    GLint attr = 0;
    if (HasExtension(GLExt::EXT_draw_buffers2))
        glGetIntegeri_v(param, index, &attr);
    return static_cast<std::uint32_t>(attr);
};

static float GLGetFloat(GLenum param)
{
    GLfloat attr = 0.0f;
    glGetFloatv(param, &attr);
    return attr;
}

static std::vector<ShadingLanguage> GLQueryShadingLanguages()
{
    std::vector<ShadingLanguage> languages;

    if (HasExtension(GLExt::ARB_shader_objects))
    {
        /* Derive shading language version by OpenGL version */
        GLint major = 0, minor = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);

        /* Map OpenGL version to GLSL version */
        const GLint version = major * 100 + minor * 10;

        /* Add supported GLSL versions */
        languages.push_back(ShadingLanguage::GLSL);

        if (version >= 200) { languages.push_back(ShadingLanguage::GLSL_110); }
        if (version >= 210) { languages.push_back(ShadingLanguage::GLSL_120); }
        if (version >= 300) { languages.push_back(ShadingLanguage::GLSL_130); }
        if (version >= 310) { languages.push_back(ShadingLanguage::GLSL_140); }
        if (version >= 320) { languages.push_back(ShadingLanguage::GLSL_150); }
        if (version >= 330) { languages.push_back(ShadingLanguage::GLSL_330); }
        if (version >= 400) { languages.push_back(ShadingLanguage::GLSL_400); }
        if (version >= 410) { languages.push_back(ShadingLanguage::GLSL_410); }
        if (version >= 420) { languages.push_back(ShadingLanguage::GLSL_420); }
        if (version >= 430) { languages.push_back(ShadingLanguage::GLSL_430); }
        if (version >= 440) { languages.push_back(ShadingLanguage::GLSL_440); }
        if (version >= 450) { languages.push_back(ShadingLanguage::GLSL_450); }
        if (version >= 460) { languages.push_back(ShadingLanguage::GLSL_460); }
    }

    #if defined GL_ARB_gl_spirv && defined GL_ARB_ES2_compatibility

    if (HasExtension(GLExt::ARB_gl_spirv) && HasExtension(GLExt::ARB_ES2_compatibility))
    {
        /* Query supported shader binary formats */
        GLint numBinaryFormats = 0;
        glGetIntegerv(GL_NUM_SHADER_BINARY_FORMATS, &numBinaryFormats);

        if (numBinaryFormats > 0)
        {
            std::vector<GLint> binaryFormats(static_cast<std::size_t>(numBinaryFormats));
            glGetIntegerv(GL_SHADER_BINARY_FORMATS, binaryFormats.data());

            if (std::find(binaryFormats.begin(), binaryFormats.end(), GL_SHADER_BINARY_FORMAT_SPIR_V) != binaryFormats.end())
            {
                /* Add supported SPIR-V versions */
                languages.push_back(ShadingLanguage::SPIRV);
                languages.push_back(ShadingLanguage::SPIRV_100);
            }
        }
    }

    #endif // /GL_ARB_gl_spirv

    return languages;
}

static std::vector<Format> GetDefaultSupportedGLTextureFormats()
{
    return
    {
        Format::R8UNorm,            Format::R8SNorm,            Format::R8UInt,             Format::R8SInt,
        Format::R16UNorm,           Format::R16SNorm,           Format::R16UInt,            Format::R16SInt,            Format::R16Float,
        Format::R32UInt,            Format::R32SInt,            Format::R32Float,
        Format::RG8UNorm,           Format::RG8SNorm,           Format::RG8UInt,            Format::RG8SInt,
        Format::RG16UNorm,          Format::RG16SNorm,          Format::RG16UInt,           Format::RG16SInt,           Format::RG16Float,
        Format::RG32UInt,           Format::RG32SInt,           Format::RG32Float,
        Format::RGB8UNorm,          Format::RGB8SNorm,          Format::RGB8UInt,           Format::RGB8SInt,
        Format::RGB16UNorm,         Format::RGB16SNorm,         Format::RGB16UInt,          Format::RGB16SInt,          Format::RGB16Float,
        Format::RGB32UInt,          Format::RGB32SInt,          Format::RGB32Float,
        Format::RGBA8UNorm,         Format::RGBA8SNorm,         Format::RGBA8UInt,          Format::RGBA8SInt,
        Format::RGBA16UNorm,        Format::RGBA16SNorm,        Format::RGBA16UInt,         Format::RGBA16SInt,         Format::RGBA16Float,
        Format::RGBA32UInt,         Format::RGBA32SInt,         Format::RGBA32Float,
        Format::BGRA8UNorm,         Format::BGRA8UNorm_sRGB,    Format::BGRA8SNorm,         Format::BGRA8UInt,          Format::BGRA8SInt,
        Format::D16UNorm,           Format::D32Float,           Format::D24UNormS8UInt,     Format::D32FloatS8X24UInt,
    };
}

static void GLGetRenderingAttribs(RenderingCapabilities& caps)
{
    /* Set fixed states for this renderer */
    caps.screenOrigin       = ScreenOrigin::LowerLeft;
    caps.clippingRange      = ClippingRange::MinusOneToOne;
    caps.shadingLanguages   = GLQueryShadingLanguages();
}

static void GLGetSupportedTextureFormats(std::vector<Format>& textureFormats)
{
    textureFormats = GetDefaultSupportedGLTextureFormats();

    #if defined GL_ARB_internalformat_query && defined GL_ARB_internalformat_query2

    if (HasExtension(GLExt::ARB_internalformat_query) && HasExtension(GLExt::ARB_internalformat_query2))
    {
        RemoveAllFromListIf(
            textureFormats,
            [](Format format) -> bool
            {
                if (auto internalformat = GLTypes::MapOrZero(format))
                {
                    GLint supported = 0;
                    glGetInternalformativ(GL_TEXTURE_2D, internalformat, GL_INTERNALFORMAT_SUPPORTED, 1, &supported);
                    return (supported == GL_FALSE);
                }
                return true;
            }
        );
    }

    #endif

    #ifdef GL_EXT_texture_compression_s3tc

    const auto numCompressedTexFormats = GLGetUInt(GL_NUM_COMPRESSED_TEXTURE_FORMATS);

    std::vector<GLint> compressedTexFormats(numCompressedTexFormats);
    glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, compressedTexFormats.data());

    for (GLint internalFormat : compressedTexFormats)
    {
        auto format = GLTypes::UnmapFormat(internalFormat);
        if (format != Format::Undefined)
            textureFormats.push_back(format);
    }

    #endif
}

static void GLGetSupportedFeatures(RenderingFeatures& features)
{
    /* Query all boolean capabilies by their respective OpenGL extension */
    features.hasDirectResourceBinding       = true;
    features.hasRenderTargets               = HasExtension(GLExt::ARB_framebuffer_object);
    features.has3DTextures                  = HasExtension(GLExt::EXT_texture3D);
    features.hasCubeTextures                = HasExtension(GLExt::ARB_texture_cube_map);
    features.hasArrayTextures               = HasExtension(GLExt::EXT_texture_array);
    features.hasCubeArrayTextures           = HasExtension(GLExt::ARB_texture_cube_map_array);
    features.hasMultiSampleTextures         = HasExtension(GLExt::ARB_texture_multisample);
    features.hasTextureViews                = HasExtension(GLExt::ARB_texture_view);
    features.hasTextureViewSwizzle          = HasExtension(GLExt::ARB_texture_view); // same as for 'hasTextureViews'
    features.hasBufferViews                 = (HasExtension(GLExt::ARB_multi_bind) || HasExtension(GLExt::EXT_transform_feedback) || HasExtension(GLExt::NV_transform_feedback));
    features.hasSamplers                    = HasExtension(GLExt::ARB_sampler_objects);
    features.hasConstantBuffers             = HasExtension(GLExt::ARB_uniform_buffer_object);
    features.hasStorageBuffers              = HasExtension(GLExt::ARB_shader_storage_buffer_object);
    features.hasUniforms                    = HasExtension(GLExt::ARB_shader_objects);
    features.hasGeometryShaders             = HasExtension(GLExt::ARB_geometry_shader4);
    features.hasTessellationShaders         = HasExtension(GLExt::ARB_tessellation_shader);
    features.hasTessellatorStage            = HasExtension(GLExt::ARB_tessellation_shader); // same as for 'hasTessellationShaders'
    features.hasComputeShaders              = HasExtension(GLExt::ARB_compute_shader);
    features.hasInstancing                  = HasExtension(GLExt::ARB_draw_instanced);
    features.hasOffsetInstancing            = HasExtension(GLExt::ARB_base_instance);
    features.hasIndirectDrawing             = HasExtension(GLExt::ARB_draw_indirect);
    features.hasViewportArrays              = HasExtension(GLExt::ARB_viewport_array);
    features.hasConservativeRasterization   = (HasExtension(GLExt::NV_conservative_raster) || HasExtension(GLExt::INTEL_conservative_rasterization));
    features.hasStreamOutputs               = (HasExtension(GLExt::EXT_transform_feedback) || HasExtension(GLExt::NV_transform_feedback));
    features.hasLogicOp                     = true;
    features.hasPipelineStatistics          = HasExtension(GLExt::ARB_pipeline_statistics_query);
    features.hasRenderCondition             = true;
}

static void GLGetFeatureLimits(const RenderingFeatures& features, RenderingLimits& limits)
{
    /* Determine minimal line width range for both aliased and smooth lines */
    GLfloat aliasedLineRange[2];
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, aliasedLineRange);

    GLfloat smoothLineRange[2];
    glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, smoothLineRange);

    limits.lineWidthRange[0]                = std::max(aliasedLineRange[0], smoothLineRange[0]);
    limits.lineWidthRange[1]                = std::min(aliasedLineRange[1], smoothLineRange[1]);

    /* Query integral attributes */
    limits.maxTextureArrayLayers            = GLGetUInt(GL_MAX_ARRAY_TEXTURE_LAYERS);
    limits.maxColorAttachments              = GLGetUInt(GL_MAX_DRAW_BUFFERS);
    limits.maxPatchVertices                 = GLGetUInt(GL_MAX_PATCH_VERTICES);
    limits.maxAnisotropy                    = static_cast<std::uint32_t>(GLGetFloat(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT));

    if (features.hasComputeShaders)
    {
        limits.maxComputeShaderWorkGroups[0]    = GLGetUIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0);
        limits.maxComputeShaderWorkGroups[1]    = GLGetUIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1);
        limits.maxComputeShaderWorkGroups[2]    = GLGetUIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2);
        limits.maxComputeShaderWorkGroupSize[0] = GLGetUIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0);
        limits.maxComputeShaderWorkGroupSize[1] = GLGetUIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1);
        limits.maxComputeShaderWorkGroupSize[2] = GLGetUIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2);
    }

    #ifdef GL_ARB_uniform_buffer_object
    if (HasExtension(GLExt::ARB_uniform_buffer_object))
        limits.minConstantBufferAlignment   = GLGetUInt(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT);
    #endif // /GL_ARB_uniform_buffer_object

    #ifdef GL_ARB_shader_storage_buffer_object
    if (HasExtension(GLExt::ARB_shader_storage_buffer_object))
    {
        limits.minSampledBufferAlignment    = GLGetUInt(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT);
        limits.minStorageBufferAlignment    = limits.minSampledBufferAlignment; // Use SSBO for both sampled and storage buffers
    }
    #endif // /GL_ARB_shader_storage_buffer_object

    /* Query viewport limits */
    limits.maxViewports                     = GLGetUInt(GL_MAX_VIEWPORTS);

    GLint maxViewportDims[2];
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, maxViewportDims);
    limits.maxViewportSize[0]               = static_cast<std::uint32_t>(maxViewportDims[0]);
    limits.maxViewportSize[1]               = static_cast<std::uint32_t>(maxViewportDims[1]);

    /* Determine maximum buffer size to maximum value for <GLsizei> (used in 'glBufferData') */
    limits.maxBufferSize                    = static_cast<std::uint64_t>(std::numeric_limits<GLsizeiptr>::max());
    limits.maxConstantBufferSize            = static_cast<std::uint64_t>(GLGetUInt(GL_MAX_UNIFORM_BLOCK_SIZE));

    /* Determine maximum number of stream-outputs */
    #ifdef GL_ARB_transform_feedback3
    if (HasExtension(GLExt::ARB_transform_feedback3))
    {
        /* Get maximum number of stream-outputs from <GL_ARB_transform_feedback3> extension */
        limits.maxStreamOutputs = GLGetUInt(GL_MAX_TRANSFORM_FEEDBACK_BUFFERS);
    }
    else
    #endif // /GL_ARB_transform_feedback3
    if (HasExtension(GLExt::EXT_transform_feedback) || HasExtension(GLExt::NV_transform_feedback))
    {
        /* Presume that at least one stream-output is supported */
        limits.maxStreamOutputs = 1u;
    }

    /* Determine tessellation limits */
    limits.maxTessFactor = GLGetUInt(GL_MAX_TESS_GEN_LEVEL);
}

static void GLGetTextureLimits(const RenderingFeatures& features, RenderingLimits& limits)
{
    /* Query maximum texture dimensions */
    GLint querySizeBase = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &querySizeBase);

    /* Query 1D texture max size */
    GLint texSize = 0;
    auto querySize = querySizeBase;

    while (texSize == 0 && querySize > 0)
    {
        glTexImage1D(GL_PROXY_TEXTURE_1D, 0, GL_RGBA, querySize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_1D, 0, GL_TEXTURE_WIDTH, &texSize);
        querySize /= 2;
    }

    limits.max1DTextureSize = static_cast<std::uint32_t>(texSize);

    /* Query 2D texture max size */
    texSize = 0;
    querySize = querySizeBase;

    while (texSize == 0 && querySize > 0)
    {
        glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, querySize, querySize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texSize);
        querySize /= 2;
    }

    limits.max2DTextureSize = static_cast<std::uint32_t>(texSize);

    /* Query 3D texture max size */
    if (features.has3DTextures)
    {
        texSize = 0;
        querySize = querySizeBase;

        while (texSize == 0 && querySize > 0)
        {
            glTexImage3D(GL_PROXY_TEXTURE_3D, 0, GL_RGBA, querySize, querySize, querySize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glGetTexLevelParameteriv(GL_PROXY_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &texSize);
            querySize /= 2;
        }

        limits.max3DTextureSize = static_cast<std::uint32_t>(texSize);
    }

    /* Query cube texture max size */
    if (features.hasCubeTextures)
    {
        texSize = 0;
        querySize = querySizeBase;

        while (texSize == 0 && querySize > 0)
        {
            glTexImage2D(GL_PROXY_TEXTURE_CUBE_MAP, 0, GL_RGBA, querySize, querySize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glGetTexLevelParameteriv(GL_PROXY_TEXTURE_CUBE_MAP, 0, GL_TEXTURE_WIDTH, &texSize);
            querySize /= 2;
        }

        limits.maxCubeTextureSize = static_cast<std::uint32_t>(texSize);
    }
}

void GLQueryRenderingCaps(RenderingCapabilities& caps)
{
    GLGetRenderingAttribs(caps);
    GLGetSupportedTextureFormats(caps.textureFormats);
    GLGetSupportedFeatures(caps.features);
    GLGetFeatureLimits(caps.features, caps.limits);
    GLGetTextureLimits(caps.features, caps.limits);
}


} // /namespace LLGL



// ================================================================================
