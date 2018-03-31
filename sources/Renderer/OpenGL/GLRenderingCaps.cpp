/*
 * GLRenderingCaps.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderingCaps.h"
#include "Ext/GLExtensions.h"
#include "../GLCommon/GLExtensionRegistry.h"
#include "../GLCommon/GLTypes.h"
#include "../../Core/Helper.h"
#include <cstdint>


namespace LLGL
{


static std::uint32_t GLGetUInt(GLenum param)
{
    GLint attr = 0;
    glGetIntegerv(param, &attr);
    return static_cast<std::uint32_t>(attr);
};

static std::uint32_t GLGetUIntIndexed(GLenum param, GLuint index)
{
    GLint attr = 0;
    if (HasExtension(GLExt::EXT_draw_buffers2))
        glGetIntegeri_v(param, index, &attr);
    return static_cast<std::uint32_t>(attr);
};

static ShadingLanguage GLQueryShadingLanguage()
{
    /* Derive shading language version by OpenGL version */
    GLint major = 0, minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    /* Map OpenGL version to GLSL version */
    const GLint version = major * 100 + minor;
    switch (version)
    {
        case 200: return ShadingLanguage::GLSL_110;
        case 210: return ShadingLanguage::GLSL_120;
        case 300: return ShadingLanguage::GLSL_130;
        case 310: return ShadingLanguage::GLSL_140;
        case 320: return ShadingLanguage::GLSL_150;
        case 330: return ShadingLanguage::GLSL_330;
        case 400: return ShadingLanguage::GLSL_400;
        case 410: return ShadingLanguage::GLSL_410;
        case 420: return ShadingLanguage::GLSL_420;
        case 430: return ShadingLanguage::GLSL_430;
        case 440: return ShadingLanguage::GLSL_440;
        case 450: return ShadingLanguage::GLSL_450;
        case 460: return ShadingLanguage::GLSL_460;
    }

    return (version > 460 ? ShadingLanguage::GLSL_460 : ShadingLanguage::Unsupported);
}

static std::vector<TextureFormat> GetDefaultSupportedGLTextureFormats()
{
    return
    {
        TextureFormat::DepthComponent,
        TextureFormat::DepthStencil,
        TextureFormat::R,
        TextureFormat::RG,
        TextureFormat::RGB,
        TextureFormat::RGBA,
        TextureFormat::R8,
        TextureFormat::R8Sgn,
        TextureFormat::R16,
        TextureFormat::R16Sgn,
        TextureFormat::R16Float,
        TextureFormat::R32UInt,
        TextureFormat::R32SInt,
        TextureFormat::R32Float,
        TextureFormat::RG8,
        TextureFormat::RG8Sgn,
        TextureFormat::RG16,
        TextureFormat::RG16Sgn,
        TextureFormat::RG16Float,
        TextureFormat::RG32UInt,
        TextureFormat::RG32SInt,
        TextureFormat::RG32Float,
        TextureFormat::RGB8,
        TextureFormat::RGB8Sgn,
        TextureFormat::RGB16,
        TextureFormat::RGB16Sgn,
        TextureFormat::RGB16Float,
        TextureFormat::RGB32UInt,
        TextureFormat::RGB32SInt,
        TextureFormat::RGB32Float,
        TextureFormat::RGBA8,
        TextureFormat::RGBA8Sgn,
        TextureFormat::RGBA16,
        TextureFormat::RGBA16Sgn,
        TextureFormat::RGBA16Float,
        TextureFormat::RGBA32UInt,
        TextureFormat::RGBA32SInt,
        TextureFormat::RGBA32Float,
    };
}

static void GLGetRenderingAttribs(RenderingCaps& caps)
{
    /* Set fixed states for this renderer */
    caps.screenOrigin       = ScreenOrigin::LowerLeft;
    caps.clippingRange      = ClippingRange::MinusOneToOne;
    caps.shadingLanguage    = GLQueryShadingLanguage();
}

static void GLGetSupportedTextureFormats(std::vector<TextureFormat>& textureFormats)
{
    textureFormats = GetDefaultSupportedGLTextureFormats();

    if (HasExtension(GLExt::ARB_internalformat_query) && HasExtension(GLExt::ARB_internalformat_query2))
    {
        RemoveAllFromListIf(
            textureFormats,
            [](TextureFormat format) -> bool
            {
                if (auto internalformat = GLTypes::MapOrZero(format))
                {
                    GLint params[1];
                    glGetInternalformativ(GL_TEXTURE_2D, internalformat, GL_INTERNALFORMAT_SUPPORTED, 1, params);
                    return (params[0] == GL_FALSE);
                }
                return true;
            }
        );
    }
    
    #ifdef GL_EXT_texture_compression_s3tc

    const auto numCompressedTexFormats = GLGetUInt(GL_NUM_COMPRESSED_TEXTURE_FORMATS);

    std::vector<GLint> compressedTexFormats(numCompressedTexFormats);
    glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, compressedTexFormats.data());

    for (GLint format : compressedTexFormats)
    {
        switch (format)
        {
            case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
                textureFormats.push_back(TextureFormat::RGB_DXT1);
                break;
            case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
                textureFormats.push_back(TextureFormat::RGBA_DXT1);
                break;
            case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
                textureFormats.push_back(TextureFormat::RGBA_DXT3);
                break;
            case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
                textureFormats.push_back(TextureFormat::RGBA_DXT5);
                break;
            default:
                break;
        }
    }

    #endif
}

static void GLGetSupportedFeatures(RenderingCaps& caps)
{
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
}

static void GLGetFeatureLimits(RenderingCaps& caps)
{
    /* Query integral attributes */
    caps.maxNumTextureArrayLayers           = GLGetUInt(GL_MAX_ARRAY_TEXTURE_LAYERS);
    caps.maxNumRenderTargetAttachments      = GLGetUInt(GL_MAX_DRAW_BUFFERS);
    caps.maxConstantBufferSize              = GLGetUInt(GL_MAX_UNIFORM_BLOCK_SIZE);
    caps.maxPatchVertices                   = GLGetUInt(GL_MAX_PATCH_VERTICES);
    caps.maxAnisotropy                      = 16; //TODO: glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT)
    
    caps.maxNumComputeShaderWorkGroups[0]   = GLGetUIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0);
    caps.maxNumComputeShaderWorkGroups[1]   = GLGetUIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1);
    caps.maxNumComputeShaderWorkGroups[2]   = GLGetUIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2);

    caps.maxComputeShaderWorkGroupSize[0]   = GLGetUIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0);
    caps.maxComputeShaderWorkGroupSize[1]   = GLGetUIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1);
    caps.maxComputeShaderWorkGroupSize[2]   = GLGetUIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2);

    /* Query viewport limits */
    caps.maxNumViewports    = GLGetUInt(GL_MAX_VIEWPORTS);

    GLint maxViewportDims[2];
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, maxViewportDims);
    caps.maxViewportSize[0] = static_cast<std::uint32_t>(maxViewportDims[0]);
    caps.maxViewportSize[1] = static_cast<std::uint32_t>(maxViewportDims[1]);
}

static void GLGetTextureLimits(RenderingCaps& caps)
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

    caps.max1DTextureSize = static_cast<std::uint32_t>(texSize);

    /* Query 2D texture max size */
    texSize = 0;
    querySize = querySizeBase;

    while (texSize == 0 && querySize > 0)
    {
        glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, querySize, querySize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texSize);
        querySize /= 2;
    }

    caps.max2DTextureSize = static_cast<std::uint32_t>(texSize);

    /* Query 3D texture max size */
    if (caps.has3DTextures)
    {
        texSize = 0;
        querySize = querySizeBase;

        while (texSize == 0 && querySize > 0)
        {
            glTexImage3D(GL_PROXY_TEXTURE_3D, 0, GL_RGBA, querySize, querySize, querySize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glGetTexLevelParameteriv(GL_PROXY_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &texSize);
            querySize /= 2;
        }

        caps.max3DTextureSize = static_cast<std::uint32_t>(texSize);
    }

    /* Query cube texture max size */
    if (caps.hasCubeTextures)
    {
        texSize = 0;
        querySize = querySizeBase;

        while (texSize == 0 && querySize > 0)
        {
            glTexImage2D(GL_PROXY_TEXTURE_CUBE_MAP, 0, GL_RGBA, querySize, querySize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glGetTexLevelParameteriv(GL_PROXY_TEXTURE_CUBE_MAP, 0, GL_TEXTURE_WIDTH, &texSize);
            querySize /= 2;
        }

        caps.maxCubeTextureSize = static_cast<std::uint32_t>(texSize);
    }
}

void GLQueryRenderingCaps(RenderingCaps& caps)
{
    GLGetRenderingAttribs(caps);
    GLGetSupportedTextureFormats(caps.textureFormats);
    GLGetSupportedFeatures(caps);
    GLGetFeatureLimits(caps);
    GLGetTextureLimits(caps);
}


} // /namespace LLGL



// ================================================================================
