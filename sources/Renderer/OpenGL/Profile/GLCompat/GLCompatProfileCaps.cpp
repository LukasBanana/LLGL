/*
 * GLCompatProfileCaps.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../../GLRenderingCaps.h"
#include "../../GLTypes.h"
#include "../../Ext/GLExtensions.h"
#include "../../Ext/GLExtensionRegistry.h"
#include "../../../../Core/CoreUtils.h"
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
    #if GL_EXT_draw_buffers2
    if (HasExtension(GLExt::EXT_draw_buffers2))
        glGetIntegeri_v(param, index, &attr);
    #endif
    return static_cast<std::uint32_t>(attr);
};

static float GLGetFloat(GLenum param)
{
    GLfloat attr = 0.0f;
    glGetFloatv(param, &attr);
    return attr;
}

static const GLubyte* ParseGLVersionInteger(const GLubyte* s, GLint& outValue)
{
    outValue = 0;
    while (*s != '\0' && *s >= '0' && *s <= '9')
    {
        GLint digit = *s - '0';
        outValue *= 10;
        outValue += digit;
        ++s;
    }
    return s;
}

static bool ParseGLVersionString(const GLubyte* s, GLint& outMajor, GLint& outMinor)
{
    /*
    GL_VERSION must return a string that starts either with <MAJOR>.<MINOR> or <MAJOR>.<MINOR>.<RELEASE> followed by vendor specific information.
    Parse only the first two integers separated by a decimal.
    */
    GLint major = 0, minor = 0;
    s = ParseGLVersionInteger(s, major);
    if (*s != '.')
        return false;
    ParseGLVersionInteger(s + 1, minor);

    /* Return version numbers */
    outMajor = major;
    outMinor = minor;
    return true;
}

static std::vector<ShadingLanguage> GLQueryShadingLanguages()
{
    std::vector<ShadingLanguage> languages;
    languages.reserve(16);

    if (HasExtension(GLExt::ARB_shader_objects))
    {
        /* Derive shading language version by OpenGL version */
        GLint major = 0, minor = 0;

        /* Fallback to parsing GL_VERSION string for GL 2.x */
        if (!ParseGLVersionString(glGetString(GL_VERSION), major, minor))
        {
            /* If parsing failed, assume default 2.0 version */
            major = 2;
            minor = 0;
        }

        /* Map OpenGL version to GLSL version */
        const GLint version = major * 100 + minor * 10;

        /* Add supported GLSL versions */
        languages.push_back(ShadingLanguage::GLSL);

        if (version >= 200) { languages.push_back(ShadingLanguage::GLSL_110); }
        if (version >= 210) { languages.push_back(ShadingLanguage::GLSL_120); }
    }

    return languages;
}

static std::vector<Format> GetDefaultSupportedGLTextureFormats()
{
    return
    {
        Format::A8UNorm,
        Format::R8UNorm,
        Format::R16UNorm,
        Format::RG8UNorm,
        Format::RG16UNorm,
        Format::RGB8UNorm,          Format::RGB8UNorm_sRGB,
        Format::RGB16UNorm,
        Format::RGBA8UNorm,
        Format::RGBA16UNorm,
        Format::BGRA8UNorm,         Format::BGRA8UNorm_sRGB,
        Format::D16UNorm,           Format::D32Float,           Format::D24UNormS8UInt,     Format::D32FloatS8X24UInt,
    };
}

static void GLGetRenderingAttribs(RenderingCapabilities& caps)
{
    /* Set fixed states for this renderer */
    caps.screenOrigin       = (HasExtension(GLExt::ARB_clip_control) ? ScreenOrigin::UpperLeft : ScreenOrigin::LowerLeft);
    caps.clippingRange      = ClippingRange::MinusOneToOne;
    caps.shadingLanguages   = GLQueryShadingLanguages();
}

static void GLGetSupportedTextureFormats(std::vector<Format>& textureFormats)
{
    textureFormats = GetDefaultSupportedGLTextureFormats();

    #if GL_ARB_internalformat_query && GL_ARB_internalformat_query2

    if (HasExtension(GLExt::ARB_internalformat_query) && HasExtension(GLExt::ARB_internalformat_query2))
    {
        RemoveAllFromListIf(
            textureFormats,
            [](Format format) -> bool
            {
                if (GLenum internalformat = GLTypes::MapOrZero(format))
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

    #if GL_EXT_texture_compression_s3tc

    const std::uint32_t numCompressedTexFormats = GLGetUInt(GL_NUM_COMPRESSED_TEXTURE_FORMATS);

    std::vector<GLint> compressedTexFormats(numCompressedTexFormats);
    glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, compressedTexFormats.data());

    for (GLint internalFormat : compressedTexFormats)
    {
        const Format format = GLTypes::UnmapFormat(internalFormat);
        if (format != Format::Undefined)
            textureFormats.push_back(format);
    }

    #endif
}

static void GLGetSupportedFeatures(RenderingFeatures& features)
{
    /* Query all boolean capabilies by their respective OpenGL extension */
    features.hasRenderTargets               = HasExtension(GLExt::ARB_framebuffer_object);
    features.has3DTextures                  = HasExtension(GLExt::EXT_texture3D);
    features.hasCubeTextures                = HasExtension(GLExt::ARB_texture_cube_map);
    features.hasArrayTextures               = false;
    features.hasCubeArrayTextures           = false;
    features.hasMultiSampleTextures         = false;
    features.hasMultiSampleArrayTextures    = false;
    features.hasTextureViews                = false;
    features.hasTextureViewSwizzle          = false;
    features.hasTextureViewFormatSwizzle    = false;
    features.hasBufferViews                 = false;
    features.hasConstantBuffers             = false;
    features.hasStorageBuffers              = false;
    features.hasGeometryShaders             = false;
    features.hasTessellationShaders         = false;
    features.hasTessellatorStage            = false;
    features.hasComputeShaders              = false;
    features.hasInstancing                  = false;
    features.hasOffsetInstancing            = false;
    features.hasIndirectDrawing             = false;
    features.hasViewportArrays              = false;
    features.hasConservativeRasterization   = false;
    features.hasStreamOutputs               = (HasExtension(GLExt::EXT_transform_feedback) || HasExtension(GLExt::NV_transform_feedback));
    features.hasLogicOp                     = true;
    features.hasPipelineStatistics          = false;
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
    #ifdef GL_MAX_COLOR_ATTACHMENTS
    limits.maxColorAttachments              = std::min<std::uint32_t>(GLGetUInt(GL_MAX_DRAW_BUFFERS), GLGetUInt(GL_MAX_COLOR_ATTACHMENTS));
    #else
    limits.maxColorAttachments              = GLGetUInt(GL_MAX_DRAW_BUFFERS);
    #endif
    limits.maxAnisotropy                    = static_cast<std::uint32_t>(GLGetFloat(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT));

    /* Query viewport limits */
    limits.maxViewports                     = 1;

    GLint maxViewportDims[2];
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, maxViewportDims);
    limits.maxViewportSize[0]               = static_cast<std::uint32_t>(maxViewportDims[0]);
    limits.maxViewportSize[1]               = static_cast<std::uint32_t>(maxViewportDims[1]);

    /* Determine maximum buffer size to maximum value for <GLsizei> (used in 'glBufferData') */
    limits.maxBufferSize                    = static_cast<std::uint64_t>(std::numeric_limits<GLsizeiptr>::max());
    #ifdef GL_MAX_UNIFORM_BLOCK_SIZE
    limits.maxConstantBufferSize            = static_cast<std::uint64_t>(GLGetUInt(GL_MAX_UNIFORM_BLOCK_SIZE));
    #endif

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
    #ifdef GL_MAX_TESS_GEN_LEVEL
    limits.maxTessFactor = GLGetUInt(GL_MAX_TESS_GEN_LEVEL);
    #endif

    /* Determine maximum number of samples for render-target attachments */
    constexpr GLuint maxSamples = 1;
    limits.maxColorBufferSamples    = maxSamples;
    limits.maxDepthBufferSamples    = maxSamples;
    limits.maxStencilBufferSamples  = maxSamples;

    /* Use maximum number of samples for color buffers as fallbacks for empty render-targets */
    limits.maxNoAttachmentSamples = limits.maxColorAttachments;
}

static void GLGetTextureLimits(const RenderingFeatures& features, RenderingLimits& limits)
{
    /* Query maximum texture dimensions */
    GLint querySizeBase = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &querySizeBase);

    /* Query 1D texture max size */
    GLint texSize = 0;
    GLint querySize = querySizeBase;

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
