/*
 * GLMipGenerator.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLMipGenerator.h"
#include "GLTexture.h"
#include "../RenderState/GLStateManager.h"
#include "../Profile/GLProfile.h"
#include "../GLTypes.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../../CheckedCast.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


GLMipGenerator& GLMipGenerator::Get()
{
    static GLMipGenerator instance;
    return instance;
}

void GLMipGenerator::Clear()
{
    mipGenerationFBOPair_.ReleaseFBOs();
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
        #if LLGL_GLEXT_TEXTURE_VIEW
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
        #endif // /LLGL_GLEXT_TEXTURE_VIEW
        if (textureGL.GetType() == TextureType::Texture3D)
        {
            /* Generate MIP-maps in default process */
            GenerateMipsForTexture(stateMngr, textureGL);
        }
        else
        {
            /* Generate MIP-maps in custom sub generation process */
            const Extent3D extent = textureGL.GetMipExtent(baseMipLevel);
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
    #if LLGL_GLEXT_DIRECT_STATE_ACCESS
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        /* Generate MIP-maps of named texture object */
        glGenerateTextureMipmap(texID);
    }
    else
    #endif // /LLGL_GLEXT_DIRECT_STATE_ACCESS
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

static void NextMipSize(GLint& s)
{
    s = std::max(1, s / 2);
}

static void BlitFramebufferLinear(GLint srcWidth, GLint srcHeight, GLint dstWidth, GLint dstHeight)
{
    glBlitFramebuffer(0, 0, srcWidth, srcHeight, 0, 0, dstWidth, dstHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

static void GenerateMipsRangeTexture1D(const Extent3D& extent, GLuint texID, GLint baseMipLevel, GLint numMipLevels)
{
    /* Get extent of base MIP level */
    GLint srcWidth = static_cast<GLint>(extent.width);

    GLint dstWidth = srcWidth;

    /* Blit current MIP level into next MIP level with linear sampling filter */
    for (GLint mipLevel = baseMipLevel; mipLevel + 1 < baseMipLevel + numMipLevels; ++mipLevel)
    {
        NextMipSize(dstWidth);

        GLProfile::FramebufferTexture1D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_1D, texID, mipLevel);
        GLProfile::FramebufferTexture1D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_1D, texID, mipLevel + 1);

        BlitFramebufferLinear(srcWidth, 1, dstWidth, 1);

        srcWidth = dstWidth;
    }
}

static void GenerateMipsRangeTexture2D(const Extent3D& extent, GLuint texID, GLenum texTarget, GLint baseMipLevel, GLint numMipLevels)
{
    /* Get extent of base MIP level */
    GLint srcWidth  = static_cast<GLint>(extent.width);
    GLint srcHeight = static_cast<GLint>(extent.height);

    GLint dstWidth  = srcWidth;
    GLint dstHeight = srcHeight;

    /* Blit current MIP level into next MIP level with linear sampling filter */
    for (GLint mipLevel = baseMipLevel; mipLevel + 1 < baseMipLevel + numMipLevels; ++mipLevel)
    {
        NextMipSize(dstWidth);
        NextMipSize(dstHeight);

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
    GLint srcWidth  = static_cast<GLint>(extent.width);
    GLint srcHeight = static_cast<GLint>(extent.height);

    GLint dstWidth  = srcWidth;
    GLint dstHeight = srcHeight;

    /* Blit current MIP level into next MIP level with linear sampling filter */
    for (GLint mipLevel = baseMipLevel; mipLevel + 1 < baseMipLevel + numMipLevels; ++mipLevel)
    {
        NextMipSize(dstWidth);
        NextMipSize(dstHeight);

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

    stateMngr.PushBoundFramebuffer(GLFramebufferTarget::ReadFramebuffer);
    stateMngr.PushBoundFramebuffer(GLFramebufferTarget::DrawFramebuffer);
    {
        /* Bind read framebuffer for current MIP level and draw framebuffer for next MIP level */
        stateMngr.BindFramebuffer(GLFramebufferTarget::ReadFramebuffer, mipGenerationFBOPair_.fbos[0]);
        stateMngr.BindFramebuffer(GLFramebufferTarget::DrawFramebuffer, mipGenerationFBOPair_.fbos[1]);

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
            {
                //TODO
            }
            break;

            case TextureType::TextureCube:
            {
                /* Generate MIP-maps for all 6 cube faces */
                for_range(cubeFace, 6u)
                    GenerateMipsRangeTexture2D(extent, texID, GLTypes::ToTextureCubeMap(cubeFace), baseMipLevel, numMipLevels);
            }
            break;

            case TextureType::Texture1DArray:
            case TextureType::Texture2DArray:
            case TextureType::TextureCubeArray:
            {
                /* Generate MIP-maps for each specified array layer */
                for_subrange(arrayLayer, baseArrayLayer, baseArrayLayer + numArrayLayers)
                    GenerateMipsRangeTextureLayer(extent, texID, baseMipLevel, numMipLevels, arrayLayer);
            }
            break;

            case TextureType::Texture2DMS:
            case TextureType::Texture2DMSArray:
            {
                // Do nothing - multisample textures don't have MIP-maps
            }
            break;
        }
    }
    stateMngr.PopBoundFramebuffer();
    stateMngr.PopBoundFramebuffer();
}

#if LLGL_GLEXT_TEXTURE_VIEW

void GLMipGenerator::GenerateMipsRangeWithTextureView(
    GLStateManager& stateMngr,
    GLTexture&      textureGL,
    GLuint          baseMipLevel,
    GLuint          numMipLevels,
    GLuint          baseArrayLayer,
    GLuint          numArrayLayers)
{
    /* Get GL texture ID and texture target */
    TextureType texType         = textureGL.GetType();
    GLuint      texID           = textureGL.GetID();
    GLenum      texTarget       = GLTypes::Map(texType);
    GLenum      internalFormat  = textureGL.GetGLInternalFormat();

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

#endif // /LLGL_GLEXT_TEXTURE_VIEW


} // /namespace LLGL



// ================================================================================
