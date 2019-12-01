/*
 * GLMipGenerator.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLMipGenerator.h"
#include "GLTexture.h"
#include "../RenderState/GLStateManager.h"
#include "../GLProfile.h"
#include "../GLTypes.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../../CheckedCast.h"


namespace LLGL
{


GLMipGenerator& GLMipGenerator::Get()
{
    static GLMipGenerator instance;
    return instance;
}

void GLMipGenerator::Clear()
{
    #ifdef LLGL_ENABLE_CUSTOM_SUB_MIPGEN
    mipGenerationFBOPair_.ReleaseFBOs();
    #endif
}

void GLMipGenerator::GenerateMips(const TextureType type)
{
    glGenerateMipmap(GLTypes::Map(type));
}

void GLMipGenerator::GenerateMipsForTexture(GLStateManager& stateMngr, GLTexture& textureGL)
{
    GenerateMipsPrimary(stateMngr, textureGL.GetID(), textureGL.GetType());
}

//TODO: support range for 3D textures
void GLMipGenerator::GenerateMipsRangeForTexture(
    GLStateManager& stateMngr,
    GLTexture&      textureGL,
    std::uint32_t   baseMipLevel,
    std::uint32_t   numMipLevels,
    std::uint32_t   baseArrayLayer,
    std::uint32_t   numArrayLayers)
{
    if (numMipLevels > 0 && numArrayLayers > 0)
    {
        #ifdef GL_ARB_texture_view
        if (HasExtension(GLExt::ARB_texture_view))
        {
            /* Generate MIP-maps in GL_ARB_texture_view extension process */
            GenerateMipsRangeWithTextureView(
                stateMngr,
                textureGL,
                static_cast<GLuint>(baseMipLevel),
                static_cast<GLuint>(numMipLevels),
                static_cast<GLuint>(baseArrayLayer),
                static_cast<GLuint>(numArrayLayers)
            );
        }
        else
        #endif // /GL_ARB_texture_view
        if (textureGL.GetType() == TextureType::Texture3D)
        {
            /* Generate MIP-maps in default process */
            GenerateMipsForTexture(stateMngr, textureGL);
        }
        else
        {
            /* Generate MIP-maps in custom sub generation process */
            auto extent = textureGL.GetMipExtent(baseMipLevel);

            GenerateMipsRangeWithFBO(
                stateMngr,
                textureGL,
                extent,
                static_cast<GLint>(baseMipLevel),
                static_cast<GLint>(numMipLevels),
                static_cast<GLint>(baseArrayLayer),
                static_cast<GLint>(numArrayLayers)
            );
        }
    }
}


/*
 * ======= Private: =======
 */

void GLMipGenerator::GenerateMipsPrimary(GLStateManager& stateMngr, GLuint texID, const TextureType texType)
{
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        /* Generate MIP-maps of named texture object */
        glGenerateTextureMipmap(texID);
    }
    else
    #endif
    {
        /* Restore previously bound texture on active layer */
        auto texTarget = GLStateManager::GetTextureTarget(texType);
        stateMngr.PushBoundTexture(texTarget);
        {
            /* Bind texture and generate MIP-maps */
            stateMngr.BindTexture(texTarget, texID);
            glGenerateMipmap(GLTypes::Map(texType));
        }
        stateMngr.PopBoundTexture();
    }
}

static void GetNextMipSize(GLint& s)
{
    s = std::max(1u, s / 2u);
}

static void BlitFramebufferLinear(GLint srcWidth, GLint srcHeight, GLint dstWidth, GLint dstHeight)
{
    glBlitFramebuffer(0, 0, srcWidth, srcHeight, 0, 0, dstWidth, dstHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

static void GenerateMipsRangeTexture1D(const Extent3D& extent, GLuint texID, GLint baseMipLevel, GLint numMipLevels)
{
    /* Get extent of base MIP level */
    auto srcWidth = static_cast<GLint>(extent.width);

    auto dstWidth = srcWidth;

    /* Blit current MIP level into next MIP level with linear sampling filter */
    for (auto mipLevel = baseMipLevel; mipLevel + 1 < baseMipLevel + numMipLevels; ++mipLevel)
    {
        GetNextMipSize(dstWidth);

        GLProfile::FramebufferTexture1D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_1D, texID, mipLevel);
        GLProfile::FramebufferTexture1D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_1D, texID, mipLevel + 1);

        BlitFramebufferLinear(srcWidth, 1, dstWidth, 1);

        srcWidth = dstWidth;
    }
}

static void GenerateMipsRangeTexture2D(const Extent3D& extent, GLuint texID, GLenum texTarget, GLint baseMipLevel, GLint numMipLevels)
{
    /* Get extent of base MIP level */
    auto srcWidth   = static_cast<GLint>(extent.width);
    auto srcHeight  = static_cast<GLint>(extent.height);

    auto dstWidth   = srcWidth;
    auto dstHeight  = srcHeight;

    /* Blit current MIP level into next MIP level with linear sampling filter */
    for (auto mipLevel = baseMipLevel; mipLevel + 1 < baseMipLevel + numMipLevels; ++mipLevel)
    {
        GetNextMipSize(dstWidth);
        GetNextMipSize(dstHeight);

        GLProfile::FramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texTarget, texID, mipLevel);
        GLProfile::FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texTarget, texID, mipLevel + 1);

        BlitFramebufferLinear(srcWidth, srcHeight, dstWidth, dstHeight);

        srcWidth    = dstWidth;
        srcHeight   = dstHeight;
    }
}

static void GenerateMipsRangeTextureLayer(const Extent3D& extent, GLuint texID, GLint baseMipLevel, GLint numMipLevels, GLint arrayLayer)
{
    /* Get extent of base MIP level */
    auto srcWidth   = static_cast<GLint>(extent.width);
    auto srcHeight  = static_cast<GLint>(extent.height);

    auto dstWidth   = srcWidth;
    auto dstHeight  = srcHeight;

    /* Blit current MIP level into next MIP level with linear sampling filter */
    for (auto mipLevel = baseMipLevel; mipLevel + 1 < baseMipLevel + numMipLevels; ++mipLevel)
    {
        GetNextMipSize(dstWidth);
        GetNextMipSize(dstHeight);

        GLProfile::FramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texID, mipLevel, arrayLayer);
        GLProfile::FramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texID, mipLevel + 1, arrayLayer);

        BlitFramebufferLinear(srcWidth, srcHeight, dstWidth, dstHeight);

        srcWidth    = dstWidth;
        srcHeight   = dstHeight;
    }
}

void GLMipGenerator::GenerateMipsRangeWithFBO(
    GLStateManager& stateMngr,
    GLTexture&      textureGL,
    const Extent3D& extent,
    GLint           baseMipLevel,
    GLint           numMipLevels,
    GLint           baseArrayLayer,
    GLint           numArrayLayers)
{
    /* Get GL texture ID and texture target */
    auto texID      = textureGL.GetID();
    auto texType    = textureGL.GetType();
    auto texTarget  = GLTypes::Map(texType);

    mipGenerationFBOPair_.CreateFBOs();

    stateMngr.PushBoundFramebuffer(GLFramebufferTarget::READ_FRAMEBUFFER);
    stateMngr.PushBoundFramebuffer(GLFramebufferTarget::DRAW_FRAMEBUFFER);
    {
        /* Bind read framebuffer for <current> MIP level, and draw framebuffer for <next> MIP level */
        stateMngr.BindFramebuffer(GLFramebufferTarget::READ_FRAMEBUFFER, mipGenerationFBOPair_.fbos[0]);
        stateMngr.BindFramebuffer(GLFramebufferTarget::DRAW_FRAMEBUFFER, mipGenerationFBOPair_.fbos[1]);

        switch (texType)
        {
            case TextureType::Texture1D:
            {
                GenerateMipsRangeTexture1D(extent, texID, baseMipLevel, numMipLevels);
            }
            break;

            case TextureType::Texture2D:
            {
                GenerateMipsRangeTexture2D(extent, texID, texTarget, baseMipLevel, numMipLevels);
            }
            break;

            case TextureType::Texture3D:
            break;

            case TextureType::TextureCube:
            {
                /* Generate MIP-maps for all 6 cube faces */
                static const GLenum g_cubeFaceTexTargets[] =
                {
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                    GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                    GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                    GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
                };

                for (std::size_t i = 0; i < 6; ++i)
                    GenerateMipsRangeTexture2D(extent, texID, g_cubeFaceTexTargets[i], baseMipLevel, numMipLevels);
            }
            break;

            case TextureType::Texture1DArray:
            case TextureType::Texture2DArray:
            case TextureType::TextureCubeArray:
            {
                /* Generate MIP-maps for each specified array layer */
                for (auto arrayLayer = baseArrayLayer; arrayLayer < baseArrayLayer + numArrayLayers; ++arrayLayer)
                    GenerateMipsRangeTextureLayer(extent, texID, baseMipLevel, numMipLevels, arrayLayer);
            }
            break;

            case TextureType::Texture2DMS:
            case TextureType::Texture2DMSArray:
            break;
        }
    }
    stateMngr.PopBoundFramebuffer();
    stateMngr.PopBoundFramebuffer();
}

#ifdef GL_ARB_texture_view

void GLMipGenerator::GenerateMipsRangeWithTextureView(
    GLStateManager& stateMngr,
    GLTexture&      textureGL,
    GLuint          baseMipLevel,
    GLuint          numMipLevels,
    GLuint          baseArrayLayer,
    GLuint          numArrayLayers)
{
    /* Get GL texture ID and texture target */
    auto texID          = textureGL.GetID();
    auto texType        = textureGL.GetType();
    auto texTarget      = GLTypes::Map(texType);
    auto internalFormat = textureGL.GetGLInternalFormat();

    /* Generate new texture to be used as view (due to immutable storage) */
    GLuint texViewID = 0;
    glGenTextures(1, &texViewID);

    /*
    Create texture view as storage alias from the specified input texture.
    Note: texture views can only be created with textures that have been allocated with glTexStorage!
    see https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_texture_view.txt
    */
    glTextureView(texViewID, texTarget, texID, internalFormat, baseMipLevel, numMipLevels, baseArrayLayer, numArrayLayers);

    /* Generate MIP-maps for texture view */
    GenerateMipsPrimary(stateMngr, texViewID, texType);

    /* Release temporary texture view */
    glDeleteTextures(1, &texViewID);
}

#endif // /GL_ARB_texture_view


/*
 * MipGenerationFBOPair structure
 */

GLMipGenerator::MipGenerationFBOPair::~MipGenerationFBOPair()
{
    ReleaseFBOs();
}

void GLMipGenerator::MipGenerationFBOPair::CreateFBOs()
{
    if (!fbos[0])
        glGenFramebuffers(2, fbos);
}

void GLMipGenerator::MipGenerationFBOPair::ReleaseFBOs()
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
