/*
 * GLESProfileCaps.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../GLRenderingCaps.h"
#include "../GLProfile.h"
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

// Returns the GLES version in the ESSL version format (e.g. 200 for GLES 2.0, 320 for GLES 3.2)
static GLint GetGLESVersion()
{
    GLint major = 0, minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    return (major * 100 + minor * 10);
}

static std::vector<ShadingLanguage> GLQueryShadingLanguages(GLint version)
{
    std::vector<ShadingLanguage> languages;

    /* Add supported GLSL versions */
    languages.push_back(ShadingLanguage::ESSL);

    if (version >= 200) { languages.push_back(ShadingLanguage::ESSL_100); }
    if (version >= 300) { languages.push_back(ShadingLanguage::ESSL_300); }
    if (version >= 310) { languages.push_back(ShadingLanguage::ESSL_310); }
    if (version >= 320) { languages.push_back(ShadingLanguage::ESSL_320); }

    return languages;
}

//TODO
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

static void GLGetRenderingAttribs(RenderingCapabilities& caps, GLint version)
{
    /* Set fixed states for this renderer */
    caps.screenOrigin       = ScreenOrigin::LowerLeft;
    caps.clippingRange      = ClippingRange::MinusOneToOne;
    caps.shadingLanguages   = GLQueryShadingLanguages(version);
}

static void GLGetSupportedTextureFormats(std::vector<Format>& textureFormats)
{
    textureFormats = GetDefaultSupportedGLTextureFormats();

    RemoveAllFromListIf(
        textureFormats,
        [](Format format) -> bool
        {
            if (auto internalformat = GLTypes::MapOrZero(format))
            {
                GLint supported = 0;
                GLProfile::GetInternalformativ(GL_TEXTURE_2D, internalformat, GL_INTERNALFORMAT_SUPPORTED, 1, &supported);
                return (supported == GL_FALSE);
            }
            return true;
        }
    );

    const auto numCompressedTexFormats = GLGetUInt(GL_NUM_COMPRESSED_TEXTURE_FORMATS);

    std::vector<GLint> compressedTexFormats(numCompressedTexFormats);
    glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, compressedTexFormats.data());

    for (GLint internalFormat : compressedTexFormats)
    {
        auto format = GLTypes::UnmapFormat(internalFormat);
        if (format != Format::Undefined)
            textureFormats.push_back(format);
    }
}

static void GLGetSupportedFeatures(RenderingFeatures& features, GLint version)
{
    /* Query all boolean capabilies by their respective OpenGL extension */
    features.hasDirectResourceBinding       = true;
    features.hasRenderTargets               = true;             // GLES 2.0
    features.has3DTextures                  = true;             // GLES 2.0
    features.hasCubeTextures                = true;             // GLES 2.0
    features.hasArrayTextures               = true;             // GLES 2.0
    features.hasCubeArrayTextures           = (version >= 320); // GLES 3.2
    features.hasMultiSampleTextures         = (version >= 310); // GLES 3.1
    features.hasTextureViews                = false;
    features.hasTextureViewSwizzle          = false;
    features.hasBufferViews                 = (version >= 300); // GLES 3.0
    features.hasSamplers                    = (version >= 300); // GLES 3.0
    features.hasConstantBuffers             = (version >= 300); // GLES 3.0
    features.hasStorageBuffers              = (version >= 300); // GLES 3.0
    features.hasUniforms                    = (version >= 200); // GLES 2.0
    features.hasGeometryShaders             = (version >= 320); // GLES 3.2
    features.hasTessellationShaders         = (version >= 320); // GLES 3.2
    features.hasTessellatorStage            = (version >= 320); // GLES 3.2
    features.hasComputeShaders              = (version >= 310); // GLES 3.1
    features.hasInstancing                  = (version >= 300); // GLES 3.0
    features.hasOffsetInstancing            = false;
    features.hasIndirectDrawing             = (version >= 310); // GLES 3.1
    features.hasViewportArrays              = false;
    features.hasConservativeRasterization   = false;
    features.hasStreamOutputs               = (version >= 300); // GLES 3.0
    features.hasLogicOp                     = false;
    features.hasPipelineStatistics          = false;
    features.hasRenderCondition             = false;
}

static void GLGetFeatureLimits(RenderingLimits& limits, GLint version)
{
    /* Determine minimal line width range for both aliased and smooth lines */
    GLfloat aliasedLineRange[2];
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, aliasedLineRange);

    //limits.lineWidthRange[0]              = ???
    //limits.lineWidthRange[1]              = ???

    /* Query integral attributes */
    limits.maxTextureArrayLayers            = GLGetUInt(GL_MAX_ARRAY_TEXTURE_LAYERS);
    limits.maxColorAttachments              = GLGetUInt(GL_MAX_DRAW_BUFFERS);
    //limits.maxPatchVertices               = ???
    //limits.maxAnisotropy                  = ???

    #ifdef GL_ES_VERSION_3_1
    limits.maxComputeShaderWorkGroups[0]    = GLGetUIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0);
    limits.maxComputeShaderWorkGroups[1]    = GLGetUIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1);
    limits.maxComputeShaderWorkGroups[2]    = GLGetUIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2);
    limits.maxComputeShaderWorkGroupSize[0] = GLGetUIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0);
    limits.maxComputeShaderWorkGroupSize[1] = GLGetUIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1);
    limits.maxComputeShaderWorkGroupSize[2] = GLGetUIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2);
    #endif

    limits.minConstantBufferAlignment       = GLGetUInt(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT);

    #ifdef GL_ES_VERSION_3_1
    limits.minSampledBufferAlignment        = GLGetUInt(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT);
    limits.minStorageBufferAlignment        = limits.minSampledBufferAlignment; // Use SSBO for both sampled and storage buffers
    #endif

    /* Query viewport limits */
    limits.maxViewports                     = 1;

    GLint maxViewportDims[2];
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, maxViewportDims);
    limits.maxViewportSize[0]               = static_cast<std::uint32_t>(maxViewportDims[0]);
    limits.maxViewportSize[1]               = static_cast<std::uint32_t>(maxViewportDims[1]);

    /* Determine maximum buffer size to maximum value for <GLsizei> (used in 'glBufferData') */
    limits.maxBufferSize                    = static_cast<std::uint64_t>(std::numeric_limits<GLsizeiptr>::max());
    limits.maxConstantBufferSize            = static_cast<std::uint64_t>(GLGetUInt(GL_MAX_UNIFORM_BLOCK_SIZE));

    /* Presume that at least one stream-output is supported */
    limits.maxStreamOutputs                 = 1u;

    #ifdef GL_ES_VERSION_3_2
    /* Determine tessellation limits */
    limits.maxTessFactor                    = GLGetUInt(GL_MAX_TESS_GEN_LEVEL);
    #endif
}

static void GLGetTextureLimits(const RenderingFeatures& features, RenderingLimits& limits, GLint version)
{
    #if 0 //TODO: determine texture limits without proxy textures for GLES
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
    #endif // /TODO
}

void GLQueryRenderingCaps(RenderingCapabilities& caps)
{
    const GLint version = GetGLESVersion();
    GLGetRenderingAttribs(caps, version);
    GLGetSupportedTextureFormats(caps.textureFormats);
    GLGetSupportedFeatures(caps.features, version);
    GLGetFeatureLimits(caps.limits, version);
    GLGetTextureLimits(caps.features, caps.limits, version);
}


} // /namespace LLGL



// ================================================================================
