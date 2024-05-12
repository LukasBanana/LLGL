/*
 * GLTexture.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLTexture.h"
#include "GLTextureViewPool.h"
#include "GLRenderbuffer.h"
#include "GLReadTextureFBO.h"
#include "GLMipGenerator.h"
#ifdef LLGL_GL_ENABLE_OPENGL2X
#   include "GL2XSampler.h"
#endif
#include "../GLTypes.h"
#include "../GLCore.h"
#include "../GLObjectUtils.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../RenderState/GLStateManager.h"
#include "../Texture/GLTexImage.h"
#include "../Texture/GLTexSubImage.h"
#include "../Texture/GLTextureSubImage.h"
#include "../../TextureUtils.h"
#include "../../../Core/Exception.h"
#include "../../../Core/CoreUtils.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


// Returns true if a GL renderbuffer is sufficient for a texture with the specified bind flags
static bool IsRenderbufferSufficient(const TextureDescriptor& desc)
{
    /*
    Renderbuffers can only be used under the following conditions:
    - Texture must be 2D or 2D-multisampled
    - Only a single MIP-map level
    - Only used as attachment
    - No initial image data is specified
    */
    const long attachmentBindFlags =
    (
        desc.bindFlags &
        (
            BindFlags::Sampled                  |
            BindFlags::Storage                  |
            BindFlags::ColorAttachment          |
            BindFlags::DepthStencilAttachment   |
            BindFlags::CopySrc                  |
            BindFlags::CopyDst
        )
    );
    return
    (
        desc.mipLevels == 1 &&
        (desc.type == TextureType::Texture2D || desc.type == TextureType::Texture2DMS) &&
        (attachmentBindFlags == BindFlags::ColorAttachment || attachmentBindFlags == BindFlags::DepthStencilAttachment) &&
        ((desc.miscFlags & MiscFlags::NoInitialData) != 0)
    );
}

// Maps the specified format to a swizzle format, or identity swizzle if texture swizzling is not necessary
static GLSwizzleFormat MapToGLSwizzleFormat(const Format format)
{
    const auto& formatDesc = GetFormatAttribs(format);
    if (formatDesc.format == ImageFormat::Alpha)
        return GLSwizzleFormat::Alpha;
    else if (formatDesc.format == ImageFormat::BGRA)
        return GLSwizzleFormat::BGRA;
    else
        return GLSwizzleFormat::RGBA;
}

GLTexture::GLTexture(const TextureDescriptor& desc) :
    Texture         { desc.type, desc.bindFlags                },
    numMipLevels_   { static_cast<GLsizei>(NumMipLevels(desc)) },
    isRenderbuffer_ { IsRenderbufferSufficient(desc)           },
    swizzleFormat_  { MapToGLSwizzleFormat(desc.format)        }
{
    if (IsRenderbuffer())
    {
        #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
        if (HasExtension(GLExt::ARB_direct_state_access))
        {
            /* Create new GL renderbuffer object */
            glCreateRenderbuffers(1, &id_);
        }
        else
        #endif
        {
            /* Create new GL renderbuffer object (must be bound to a target before it can be used) */
            glGenRenderbuffers(1, &id_);
        }
    }
    else
    {
        #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
        if (HasExtension(GLExt::ARB_direct_state_access))
        {
            /* Create new GL texture object with respective target */
            glCreateTextures(GetGLTexTarget(), 1, &id_);
        }
        else
        #endif
        {
            /* Create new GL texture object (must be bound to a target before it can be used) */
            glGenTextures(1, &id_);
        }
    }

    #ifdef LLGL_OPENGLES3
    /* Store additional parameters for GLES */
    extent_[0]  = static_cast<GLint>(desc.extent.width);
    extent_[1]  = static_cast<GLint>(desc.extent.height);
    extent_[2]  = static_cast<GLint>(desc.extent.depth);
    samples_    = static_cast<GLint>(desc.samples);
    #endif

    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

GLTexture::~GLTexture()
{
    if (IsRenderbuffer())
    {
        /* Delete renderbuffer and notify state manager */
        GLStateManager::Get().DeleteRenderbuffer(id_);
    }
    else
    {
        /* Delete texture and notify state manager as well as texture-view pool since this could be the source for a texture-view */
        GLStateManager::Get().DeleteTexture(id_, GLStateManager::GetTextureTarget(GetType()));
        GLTextureViewPool::Get().NotifyTextureRelease(id_);
    }
}

void GLTexture::SetDebugName(const char* name)
{
    if (IsRenderbuffer())
        GLSetObjectLabel(GL_RENDERBUFFER, GetID(), name);
    else
        GLSetObjectLabel(GL_TEXTURE, GetID(), name);
}

// Map TextureType to GLenum for glGetTexLevelParameter* functions. This is different for cube maps.
static GLenum GLGetTextureLevelParamTarget(const TextureType type)
{
    /*
    The spec. is wrong here regarding TextureCubeArray:
    GL_TEXTURE_CUBE_MAP_ARRAY is supposedly not allowed for glGetTexLevelParameter,
    but all tested GL implementations either accept it or fail with individual cube faces.
    */
    switch (type)
    {
        case TextureType::Texture1D:        return GL_TEXTURE_1D;
        case TextureType::Texture2D:        return GL_TEXTURE_2D;
        case TextureType::Texture3D:        return GL_TEXTURE_3D;
        case TextureType::TextureCube:      return GL_TEXTURE_CUBE_MAP_POSITIVE_X; // Use first cube face instead of texture type
        case TextureType::Texture1DArray:   return GL_TEXTURE_1D_ARRAY;
        case TextureType::Texture2DArray:   return GL_TEXTURE_2D_ARRAY;
        case TextureType::TextureCubeArray: return GL_TEXTURE_CUBE_MAP_ARRAY;
        case TextureType::Texture2DMS:      return GL_TEXTURE_2D_MULTISAMPLE;
        case TextureType::Texture2DMSArray: return GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
    }
    return 0;
}

Extent3D GLTexture::GetMipExtent(std::uint32_t mipLevel) const
{
    GLint texSize[3] = { 0 };
    GLint level = static_cast<GLint>(mipLevel);

    if (IsRenderbuffer())
    {
        /* Get MIP-map extent from renderbuffer object, but only for the first MIP-map */
        if (level == 0)
            GetRenderbufferSize(texSize);
    }
    else
    {
        /* Get MIP-map extent from texture object */
        GetTextureMipSize(level, texSize);
    }

    return
    {
        static_cast<std::uint32_t>(texSize[0]),
        static_cast<std::uint32_t>(texSize[1]),
        static_cast<std::uint32_t>(texSize[2])
    };
}

TextureDescriptor GLTexture::GetDesc() const
{
    TextureDescriptor texDesc;

    texDesc.type        = GetType();
    texDesc.bindFlags   = GetBindFlags();
    texDesc.format      = GetFormat();
    texDesc.mipLevels   = static_cast<std::uint32_t>(GetNumMipLevels());

    /* Query hardware texture format and size */
    GLint extent[3] = {}, samples = 1;
    if (IsRenderbuffer())
        GetRenderbufferParams(extent, &samples);
    else
        GetTextureParams(extent, &samples);

    /* Initial value of GL_TEXTURE_SAMPLES is 0, so clamp to [1, inf+) to be uniform with all backends */
    texDesc.samples = static_cast<std::uint32_t>(std::max(1, samples));

    switch (GetType())
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            texDesc.extent.width    = static_cast<std::uint32_t>(extent[0]);
            texDesc.arrayLayers     = static_cast<std::uint32_t>(extent[1]);
            break;

        case TextureType::TextureCube:
            texDesc.extent.width    = static_cast<std::uint32_t>(extent[0]);
            texDesc.extent.height   = static_cast<std::uint32_t>(extent[1]);
            texDesc.arrayLayers     = 6u;
            break;

        case TextureType::Texture2D:
        case TextureType::Texture2DArray:
        case TextureType::TextureCubeArray:
        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            /* For cube array textures, depth extent can also be copied directly without transformation (no need to multiply by 6) */
            texDesc.extent.width    = static_cast<std::uint32_t>(extent[0]);
            texDesc.extent.height   = static_cast<std::uint32_t>(extent[1]);
            texDesc.arrayLayers     = static_cast<std::uint32_t>(extent[2]);
            break;

        case TextureType::Texture3D:
            texDesc.extent.width    = static_cast<std::uint32_t>(extent[0]);
            texDesc.extent.height   = static_cast<std::uint32_t>(extent[1]);
            texDesc.extent.depth    = static_cast<std::uint32_t>(extent[2]);
            break;
    }

    return texDesc;
}

// Maps the format from Alpha swizzling to RGBA
static Format MapGLSwizzleFormatAlpha(const Format format)
{
    switch (format)
    {
        case Format::R8UNorm:   return Format::A8UNorm;
        default:                return format;
    }
}

// Maps the format from BGRA swizzling to RGBA
static Format MapGLSwizzleFormatBGRA(const Format format)
{
    switch (format)
    {
        case Format::RGBA8UNorm:        return Format::BGRA8UNorm;
        case Format::RGBA8UNorm_sRGB:   return Format::BGRA8UNorm_sRGB;
        case Format::RGBA8SNorm:        return Format::BGRA8SNorm;
        case Format::RGBA8UInt:         return Format::BGRA8UInt;
        case Format::RGBA8SInt:         return Format::BGRA8SInt;
        default:                        return format;
    }
}

// Returns the texture format for the specified texture swizzling
static Format MapGLSwizzleFormat(const Format format, const GLSwizzleFormat swizzle)
{
    switch (swizzle)
    {
        case GLSwizzleFormat::Alpha:    return MapGLSwizzleFormatAlpha(format);
        case GLSwizzleFormat::BGRA:     return MapGLSwizzleFormatBGRA(format);
        default:                        return format;
    }
}

Format GLTexture::GetFormat() const
{
    /* Translate internal format depending on texture swizzle to circumvent certain inverted formats (e.g. BGRA) */
    const Format format = GLTypes::UnmapFormat(GetGLInternalFormat());
    return MapGLSwizzleFormat(format, swizzleFormat_);
}

SubresourceFootprint GLTexture::GetSubresourceFootprint(std::uint32_t mipLevel) const
{
    const TextureDescriptor desc = GetDesc();
    return CalcPackedSubresourceFootprint(desc.type, desc.format, desc.extent, mipLevel, desc.arrayLayers);
}

static GLint GetGlTextureMinFilter(const TextureDescriptor& textureDesc)
{
    if (IsMipMappedTexture(textureDesc))
        return GL_LINEAR_MIPMAP_LINEAR;
    else
        return GL_LINEAR;
}

static ImageFormat MapSwizzleImageFormat(const ImageFormat format)
{
    switch (format)
    {
        case ImageFormat::RGBA: return ImageFormat::BGRA;
        case ImageFormat::RGB:  return ImageFormat::BGR;
        default:                return format;
    }
}

void GLTexture::BindAndAllocStorage(const TextureDescriptor& textureDesc, const ImageView* initialImage)
{
    /* Allocate texture or renderbuffer storage */
    if (IsRenderbuffer())
        AllocRenderbufferStorage(textureDesc);
    else
        AllocTextureStorage(textureDesc, initialImage);
}

static TextureSwizzle GetTextureSwizzlePermutationBGRAComponent(const TextureSwizzle swizzleComponent)
{
    switch (swizzleComponent)
    {
        case TextureSwizzle::Red:   return TextureSwizzle::Blue;    // Swap red with blue component
        case TextureSwizzle::Blue:  return TextureSwizzle::Red;     // Swap blue with red component
        default:                    return swizzleComponent;        // Use input value for all other components
    }
}

static TextureSwizzleRGBA GetTextureSwizzlePermutationBGRA(const TextureSwizzleRGBA& swizzle)
{
    TextureSwizzleRGBA permutation;
    {
        permutation.r = GetTextureSwizzlePermutationBGRAComponent(swizzle.r);
        permutation.g = GetTextureSwizzlePermutationBGRAComponent(swizzle.g);
        permutation.b = GetTextureSwizzlePermutationBGRAComponent(swizzle.b);
        permutation.a = GetTextureSwizzlePermutationBGRAComponent(swizzle.a);
    }
    return permutation;
}

// Maps the TextureSwizzleRGBA::a component to a different value for the "Alpha" swizzle format
static TextureSwizzle GetTextureSwizzlePermutationAlphaComponent(const TextureSwizzle swizzleAlpha)
{
    switch (swizzleAlpha)
    {
        case TextureSwizzle::Zero:  return TextureSwizzle::Zero;    // Zero is allowed as fixed value
        case TextureSwizzle::One:   return TextureSwizzle::One;     // One is allowed as fixed value
        case TextureSwizzle::Alpha: return TextureSwizzle::Red;     // Only alpha component can be mapped to another component
        default:                    return TextureSwizzle::Zero;    // Use zero as default value
    }
}

static TextureSwizzleRGBA GetTextureSwizzlePermutationAlpha(const TextureSwizzleRGBA& swizzle)
{
    TextureSwizzleRGBA permutation;
    {
        permutation.r = TextureSwizzle::Zero;
        permutation.g = TextureSwizzle::Zero;
        permutation.b = TextureSwizzle::Zero;
        permutation.a = GetTextureSwizzlePermutationAlphaComponent(swizzle.a);
    }
    return permutation;
}

static void InitializeGLTextureSwizzle(GLenum target, const TextureSwizzleRGBA& swizzle)
{
    glTexParameteri(target, GL_TEXTURE_SWIZZLE_R, GLTypes::Map(swizzle.r));
    glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, GLTypes::Map(swizzle.g));
    glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, GLTypes::Map(swizzle.b));
    glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, GLTypes::Map(swizzle.a));
}

static void InitializeGLTextureSwizzleWithFormat(
    const TextureType           type,
    const GLSwizzleFormat       swizzleFormat,
    const TextureSwizzleRGBA&   swizzle,
    bool                        ignoreIdentitySwizzle)
{
    /* Ignore initialization if default values can be used */
    if (swizzleFormat == GLSwizzleFormat::RGBA && ignoreIdentitySwizzle)
        return;

    /* Map swizzle parameters according for permutation format */
    const auto target = GLTypes::Map(type);
    switch (swizzleFormat)
    {
        case GLSwizzleFormat::RGBA:
            InitializeGLTextureSwizzle(target, swizzle);
            break;

        case GLSwizzleFormat::BGRA:
            InitializeGLTextureSwizzle(target, GetTextureSwizzlePermutationBGRA(swizzle));
            break;

        case GLSwizzleFormat::Alpha:
            InitializeGLTextureSwizzle(target, GetTextureSwizzlePermutationAlpha(swizzle));
            break;
    }
}

void GLTexture::TexParameterSwizzle(
    const TextureType           type,
    const Format                format,
    const TextureSwizzleRGBA&   swizzle,
    bool                        ignoreIdentitySwizzle)
{
    InitializeGLTextureSwizzleWithFormat(type, MapToGLSwizzleFormat(format), swizzle, ignoreIdentitySwizzle);
}

#ifdef GL_ARB_copy_image

// For glCopyImageSubData, the array lazer is always specified in the Z-coordinate
static Offset3D ToGLArrayTextureOffset(TextureType type, const Offset3D& offset)
{
    if (type == TextureType::Texture1DArray)
        return Offset3D{ offset.x, 0, offset.y };
    else
        return offset;
}

static void GLCopyImageSubData(
    GLTexture&      dstTexture,
    GLint           dstLevel,
    const Offset3D& dstOffset,
    GLTexture&      srcTexture,
    GLint           srcLevel,
    const Offset3D& srcOffset,
    const Extent3D& extent)
{
    /* Copy raw data of texture directly (GL 4.3+) */
    const Offset3D dstOffsetGL = ToGLArrayTextureOffset(dstTexture.GetType(), dstOffset);
    const Offset3D srcOffsetGL = ToGLArrayTextureOffset(srcTexture.GetType(), srcOffset);
    glCopyImageSubData(
        srcTexture.GetID(),
        GLTypes::Map(srcTexture.GetType()),
        srcLevel,
        srcOffsetGL.x,
        srcOffsetGL.y,
        srcOffsetGL.z,
        dstTexture.GetID(),
        GLTypes::Map(dstTexture.GetType()),
        dstLevel,
        dstOffsetGL.x,
        dstOffsetGL.y,
        dstOffsetGL.z,
        static_cast<GLsizei>(extent.width),
        static_cast<GLsizei>(extent.height),
        static_cast<GLsizei>(extent.depth)
    );
}

#endif // /GL_ARB_copy_image

// Copies image data from a source texture to a destination texture.
static void GLCopyTexSubImagePrimary(
    const TextureType       type,
    const GLTextureTarget   target,
    GLuint                  dstTextureID,
    GLint                   dstLevel,
    const Offset3D&         dstOffset,
    GLTexture&              srcTexture,
    GLint                   srcLevel,
    Offset3D                srcOffset,
    const Extent3D&         extent)
{
    const GLenum targetGL = GLTypes::Map(type);

    /* Create temporary FBO for source texture to read from GL_READ_FRAMEBUFFER in copy texture operator */
    GLReadTextureFBO readFBO;
    GLStateManager::Get().BindTexture(target, dstTextureID);

    switch (type)
    {
        case TextureType::Texture1D:
        {
            #ifdef LLGL_OPENGL
            readFBO.Attach(srcTexture, srcLevel, srcOffset);
            glCopyTexSubImage1D(
                targetGL,
                dstLevel,
                dstOffset.x,
                srcOffset.x,
                0,
                static_cast<GLsizei>(extent.width)
            );
            #endif
        }
        break;

        case TextureType::Texture1DArray:
        {
            for_range(y, extent.height)
            {
                readFBO.Attach(srcTexture, srcLevel, srcOffset);
                glCopyTexSubImage2D(
                    targetGL,
                    dstLevel,
                    dstOffset.x,
                    dstOffset.y + y,
                    srcOffset.x,
                    0, // y
                    static_cast<GLsizei>(extent.width),
                    1 // height
                );
                srcOffset.y++;
            }
        }
        break;

        case TextureType::Texture2D:
        case TextureType::Texture2DMS:
        {
            readFBO.Attach(srcTexture, srcLevel, srcOffset);
            glCopyTexSubImage2D(
                targetGL,
                dstLevel,
                dstOffset.x,
                dstOffset.y,
                srcOffset.x,
                srcOffset.y,
                static_cast<GLsizei>(extent.width),
                static_cast<GLsizei>(extent.height)
            );
        }
        break;

        case TextureType::TextureCube:
        {
            for_range(z, extent.depth)
            {
                readFBO.Attach(srcTexture, srcLevel, srcOffset);
                glCopyTexSubImage2D(
                    GLTypes::ToTextureCubeMap(dstOffset.z + z),
                    dstLevel,
                    dstOffset.x,
                    dstOffset.y,
                    srcOffset.x,
                    srcOffset.y,
                    static_cast<GLsizei>(extent.width),
                    static_cast<GLsizei>(extent.height)
                );
                srcOffset.z++;
            }
        }
        break;

        case TextureType::Texture3D:
        case TextureType::Texture2DArray:
        case TextureType::Texture2DMSArray:
        case TextureType::TextureCubeArray:
        {
            for_range(z, extent.depth)
            {
                readFBO.Attach(srcTexture, srcLevel, srcOffset);
                glCopyTexSubImage3D(
                    targetGL,
                    dstLevel,
                    dstOffset.x,
                    dstOffset.y,
                    dstOffset.z + z,
                    srcOffset.x,
                    srcOffset.y,
                    static_cast<GLsizei>(extent.width),
                    static_cast<GLsizei>(extent.height)
                );
                srcOffset.z++;
            }
        }
        break;
    }
}

static void GLCopyTexSubImage(
    GLTexture&      dstTexture,
    GLint           dstLevel,
    const Offset3D& dstOffset,
    GLTexture&      srcTexture,
    GLint           srcLevel,
    Offset3D        srcOffset,
    const Extent3D& extent)
{
    const TextureType       type    = dstTexture.GetType();
    const GLTextureTarget   target  = GLStateManager::GetTextureTarget(type);

    /* Store bound texture and framebuffer */
    GLStateManager::Get().PushBoundTexture(target);
    GLStateManager::Get().PushBoundFramebuffer(GLFramebufferTarget::ReadFramebuffer);
    {
        GLCopyTexSubImagePrimary(
            type,
            target,
            dstTexture.GetID(),
            dstLevel,
            dstOffset,
            srcTexture,
            srcLevel,
            srcOffset,
            extent
        );
    }
    /* Restore previous bound texture and framebuffer */
    GLStateManager::Get().PopBoundFramebuffer();
    GLStateManager::Get().PopBoundTexture();
}

void GLTexture::CopyImageSubData(
    GLint           dstLevel,
    const Offset3D& dstOffset,
    GLTexture&      srcTexture,
    GLint           srcLevel,
    const Offset3D& srcOffset,
    const Extent3D& extent)
{
    if (!IsRenderbuffer())
    {
        #ifdef GL_ARB_copy_image
        if (HasExtension(GLExt::ARB_copy_image))
        {
            /* Copy raw data of texture directly (GL 4.3+) */
            GLCopyImageSubData(*this, dstLevel, dstOffset, srcTexture, srcLevel, srcOffset, extent);
        }
        else
        #endif // /GL_ARB_copy_image
        {
            /* Copy source texture from GL_READ_BUFFER into destination texture */
            GLCopyTexSubImage(*this, dstLevel, dstOffset, srcTexture, srcLevel, srcOffset, extent);
        }
    }
}

void GLTexture::CopyImageToBuffer(
    const TextureRegion&    region,
    GLuint                  bufferID,
    GLintptr                offset,
    GLsizei                 size,
    GLint                   rowLength,
    GLint                   imageHeight)
{
    /* Get image format and data type from internal texture format */
    const auto& formatAttribs = GetFormatAttribs(GetFormat());

    /* Read data from unpack buffer with byte offset and equal texture format */
    const MutableImageView dstImageView
    {
        formatAttribs.format,
        formatAttribs.dataType,
        reinterpret_cast<void*>(offset),
        static_cast<std::size_t>(size)
    };

    /* Bind buffer to pixel transfer pack buffer unit */
    GLStateManager::Get().BindBuffer(GLBufferTarget::PixelPackBuffer, bufferID);
    GLStateManager::Get().SetPixelStorePack(rowLength, imageHeight, 1);
    {
        /* Read image sub data and store into currently bound pack buffer */
        GetTextureSubImage(region, dstImageView);
    }
    GLStateManager::Get().SetPixelStorePack(0, 0, 1);
    GLStateManager::Get().BindBuffer(GLBufferTarget::PixelPackBuffer, 0);
}

/*
TODO:
Only unbind GL_PIXEL_UNPACK_BUFFER in GLTexImage and GLTexSubImage functions
or at each other GL command that is affected by pixel transfer functions,
to avoid unnecessary binding and unbinding.
*/
void GLTexture::CopyImageFromBuffer(
    const TextureRegion&    region,
    GLuint                  bufferID,
    GLintptr                offset,
    GLsizei                 size,
    GLint                   rowLength,
    GLint                   imageHeight)
{
    /* Get image format and data type from internal texture format */
    const auto& formatAttribs = GetFormatAttribs(GetFormat());

    /* Read data from unpack buffer with byte offset and equal texture format */
    const ImageView srcImageView
    {
        formatAttribs.format,
        formatAttribs.dataType,
        reinterpret_cast<const void*>(offset),
        static_cast<std::size_t>(size)
    };

    /* Bind buffer to pixel transfer unpack buffer unit */
    GLStateManager::Get().BindBuffer(GLBufferTarget::PixelUnpackBuffer, bufferID);
    GLStateManager::Get().SetPixelStoreUnpack(rowLength, imageHeight, 1);
    {
        /* Write image sub data from currently bound unpack buffer */
        TextureSubImage(region, srcImageView);
    }
    GLStateManager::Get().SetPixelStoreUnpack(0, 0, 1);
    GLStateManager::Get().BindBuffer(GLBufferTarget::PixelUnpackBuffer, 0);
}

void GLTexture::TextureSubImage(const TextureRegion& region, const ImageView& srcImageView, bool restoreBoundTexture)
{
    if (!IsRenderbuffer())
    {
        #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
        if (HasExtension(GLExt::ARB_direct_state_access))
        {
            /* Transfer image data directly to GL texture */
            GLTextureSubImage(GetID(), GetType(), region, srcImageView, GetGLInternalFormat());
        }
        else
        #endif
        {
            const GLTextureTarget target = GLStateManager::GetTextureTarget(GetType());
            if (restoreBoundTexture)
            {
                /* Bind texture and transfer image data to GL texture, then restore previously bound texture with state manager */
                GLStateManager::Get().PushBoundTexture(target);
                {
                    GLStateManager::Get().BindTexture(target, GetID());
                    GLTexSubImage(GetType(), region, srcImageView, GetGLInternalFormat());
                }
                GLStateManager::Get().PopBoundTexture();
            }
            else
            {
                /* Bind texture and transfer image data to GL texture */
                GLStateManager::Get().BindTexture(target, GetID());
                GLTexSubImage(GetType(), region, srcImageView, GetGLInternalFormat());
            }
        }
    }
}

#ifdef GL_ARB_get_texture_sub_image

static void GLGetTextureSubImage(
    GLTexture&                  textureGL,
    const TextureRegion&        region,
    const MutableImageView&     dstImageView)
{
    /* Translate source region into actual texture dimensions */
    const auto type         = textureGL.GetType();
    const auto offset       = CalcTextureOffset(type, region.offset, region.subresource.baseArrayLayer);
    const auto extent       = CalcTextureExtent(type, region.extent, region.subresource.numArrayLayers);
    const auto mipLevel     = static_cast<GLint>(region.subresource.baseMipLevel);

    /* Get image data from texture region with native GL command */
    glGetTextureSubImage(
        textureGL.GetID(),
        mipLevel,
        static_cast<GLint>(offset.x),
        static_cast<GLint>(offset.y),
        static_cast<GLint>(offset.z),
        static_cast<GLsizei>(extent.width),
        static_cast<GLsizei>(extent.height),
        static_cast<GLsizei>(extent.depth),
        GLTypes::Map(dstImageView.format, IsIntDataType(dstImageView.dataType)),
        GLTypes::Map(dstImageView.dataType),
        static_cast<GLsizei>(dstImageView.dataSize),
        dstImageView.data
    );
}

#endif // /GL_ARB_get_texture_sub_image

#ifdef LLGL_OPENGL

// Forwards the call to glGetTexImage() and converts the output data if necessary
static void GLGetTexImage(
    TextureType type,
    GLint       mipLevel,
    ImageFormat format,
    DataType    dataType,
    void*       data,
    std::size_t numTexels)
{
    if (format == ImageFormat::Stencil && GLGetVersion() < 440)
    {
        /* GL_STENCIL_INDEX can only be passed into glGetTexImage in GL 4.4+, so read GL_DEPTH_STENCIL and separate stencil manually */
        std::unique_ptr<GLDepthStencilPair[]> dsImageData = MakeUniqueArray<GLDepthStencilPair>(numTexels);

        glGetTexImage(
            GLTypes::Map(type),
            mipLevel,
            GL_DEPTH_STENCIL,
            GL_FLOAT_32_UNSIGNED_INT_24_8_REV,
            dsImageData.get()
        );

        /* Copy stencil values into output buffer */
        std::uint8_t* dst = reinterpret_cast<std::uint8_t*>(data);
        for_range(i, numTexels)
            dst[i] = dsImageData[i].stencil;
    }
    else
    {
        glGetTexImage(
            GLTypes::Map(type),
            mipLevel,
            GLTypes::Map(format, IsIntDataType(dataType)),
            GLTypes::Map(dataType),
            data
        );
    }
}

static void GLGetTextureImage(
    GLTexture&                  textureGL,
    const TextureRegion&        region,
    const MutableImageView&     dstImageView)
{
    /* Get texture type and texture unit target */
    const TextureType type = textureGL.GetType();

    /* Translate source region into actual texture dimensions */
    const Offset3D offset = CalcTextureOffset(type, region.offset, region.subresource.baseArrayLayer);
    const Extent3D extent = CalcTextureExtent(type, region.extent, region.subresource.numArrayLayers);

    /* Check if source region must be copied into a staging texture */
    const GLint         mipLevel    = static_cast<GLint>(region.subresource.baseMipLevel);
    const Extent3D      mipExtent   = textureGL.GetMipExtent(region.subresource.baseMipLevel);
    const bool          useStaging  = (mipExtent != extent);
    const std::uint32_t numTexels   = extent.width * extent.height * extent.depth;

    if (useStaging)
    {
        /* Generate temporary staging texture */
        GLuint stagingTextureID = 0;
        glGenTextures(1, &stagingTextureID);

        /*
        Translate cube maps to 2D arrays for staging texture
        since cube-map sampling is not required and we might not have all 6 cube faces.
        */
        const TextureType       stagingTextureType      = (IsCubeTexture(type) ? TextureType::Texture2DArray : type);
        const GLTextureTarget   stagingTextureTarget    = GLStateManager::GetTextureTarget(stagingTextureType);

        /* Allocate storage for temporary staging texture */
        TextureDescriptor stagingTextureDesc;
        {
            stagingTextureDesc.type         = stagingTextureType;
            stagingTextureDesc.bindFlags    = BindFlags::CopySrc | BindFlags::CopyDst;
            stagingTextureDesc.miscFlags    = MiscFlags::NoInitialData;
            stagingTextureDesc.format       = textureGL.GetFormat();
            stagingTextureDesc.extent       = region.extent;
            stagingTextureDesc.arrayLayers  = region.subresource.numArrayLayers;
            stagingTextureDesc.mipLevels    = 1;
        };
        GLStateManager::Get().BindTexture(stagingTextureTarget, stagingTextureID);
        GLTexImage(stagingTextureDesc, nullptr);

        /* Copy source texture region into temporary staging texture */
        GLStateManager::Get().PushBoundFramebuffer(GLFramebufferTarget::ReadFramebuffer);
        {
            GLCopyTexSubImagePrimary(
                stagingTextureType,
                stagingTextureTarget,
                stagingTextureID,
                0,
                Offset3D{ 0, 0, 0 },
                textureGL,
                mipLevel,
                offset,
                extent
            );
        }
        GLStateManager::Get().PopBoundFramebuffer();

        /* Use staging texture as source for copy operation, so also reset source MIP-map level */
        #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
        if (HasExtension(GLExt::ARB_direct_state_access))
        {
            glGetTextureImage(
                stagingTextureID,
                0,
                GLTypes::Map(dstImageView.format, IsIntDataType(dstImageView.dataType)),
                GLTypes::Map(dstImageView.dataType),
                static_cast<GLsizei>(dstImageView.dataSize),
                dstImageView.data
            );
        }
        else
        #endif // /GL_ARB_direct_state_access
        {
            /* Bind texture and read image data from texture */
            GLStateManager::Get().BindTexture(stagingTextureTarget, stagingTextureID);
            GLGetTexImage(stagingTextureType, 0, dstImageView.format, dstImageView.dataType, dstImageView.data, numTexels);
        }

        /* Delete temporary staging texture */
        GLStateManager::Get().DeleteTexture(stagingTextureID, stagingTextureTarget, /*invalidateActiveLayerOnly:*/ true);
    }
    else
    {
        /* Use input texture as source for copy operation */
        const GLuint srcTextureID = textureGL.GetID();

        #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
        if (HasExtension(GLExt::ARB_direct_state_access))
        {
            glGetTextureImage(
                srcTextureID,
                mipLevel,
                GLTypes::Map(dstImageView.format, IsIntDataType(dstImageView.dataType)),
                GLTypes::Map(dstImageView.dataType),
                static_cast<GLsizei>(dstImageView.dataSize),
                dstImageView.data
            );
        }
        else
        #endif // /GL_ARB_direct_state_access
        {
            /* Bind texture and read image data from texture */
            const GLTextureTarget srcTextureTarget = GLStateManager::GetTextureTarget(type);
            GLStateManager::Get().BindTexture(srcTextureTarget, srcTextureID);
            GLGetTexImage(type, mipLevel, dstImageView.format, dstImageView.dataType, dstImageView.data, numTexels);
        }
    }
}

#endif // /LLGL_OPENGL

void GLTexture::GetTextureSubImage(const TextureRegion& region, const MutableImageView& dstImageView, bool restoreBoundTexture)
{
    if (!IsRenderbuffer())
    {
        #ifdef LLGL_OPENGL

        #ifdef GL_ARB_get_texture_sub_image
        if (HasExtension(GLExt::ARB_get_texture_sub_image))
        {
            /* Use native function to retrieve sub image data */
            GLGetTextureSubImage(*this, region, dstImageView);
        }
        else
        #endif // /GL_ARB_get_texture_sub_image
        {
            /* Emulate functionality by copying the entire texture image into an intermediate buffer */
            const GLTextureTarget target = GLStateManager::GetTextureTarget(GetType());
            if (restoreBoundTexture)
            {
                /* Bind texture and transfer image data to GL texture, then restore previously bound texture with state manager */
                GLStateManager::Get().PushBoundTexture(target);
                {
                    GLGetTextureImage(*this, region, dstImageView);
                }
                GLStateManager::Get().PopBoundTexture();
            }
            else
                GLGetTextureImage(*this, region, dstImageView);
        }

        #else

        //TODO: copy texture to unpack buffer, then map buffer range to CPU memory
        LLGL_TRAP_NOT_IMPLEMENTED();

        #endif // /LLGL_OPENGL
    }
}

GLenum GLTexture::GetGLTexTarget() const
{
    return GLTypes::Map(GetType());
}

GLenum GLTexture::GetGLTexLevelTarget() const
{
    return GLGetTextureLevelParamTarget(GetType());
}

#ifdef LLGL_GL_ENABLE_OPENGL2X

void GLTexture::BindTexParameters(const GL2XSampler& sampler)
{
    if (&sampler != boundSampler_)
    {
        sampler.BindTexParameters(GetGLTexTarget(), boundSampler_);
        boundSampler_ = &sampler;
    }
}

#endif // /LLGL_GL_ENABLE_OPENGL2X


/*
 * ======= Private: =======
 */

void GLTexture::AllocTextureStorage(const TextureDescriptor& textureDesc, const ImageView* initialImage)
{
    /* Bind texture */
    GLStateManager::Get().BindGLTexture(*this);

    /* Initialize texture parameters for the first time (sampler states not supported for multisample textures) */
    if (!IsMultiSampleTexture(textureDesc.type))
    {
        GLenum target = GLTypes::Map(textureDesc.type);
        if (IsIntegralFormat(textureDesc.format))
        {
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        else
        {
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GetGlTextureMinFilter(textureDesc));
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
    }

    /* Configure texture swizzling if format is not supported */
    InitializeGLTextureSwizzleWithFormat(GetType(), swizzleFormat_, {}, true);

    /* Convert initial image data for texture swizzle formats */
    ImageView intermediateImageView;

    if (initialImage != nullptr && GetSwizzleFormat() == GLSwizzleFormat::BGRA)
    {
        intermediateImageView = *initialImage;
        intermediateImageView.format = MapSwizzleImageFormat(initialImage->format);
        initialImage = &intermediateImageView;
    }

    /* Build texture storage and upload image dataa */
    //GLStateManager::Get().BindBuffer(GLBufferTarget::PIXEL_UNPACK_BUFFER, 0);
    GLTexImage(textureDesc, initialImage);

    /* Store internal GL format */
    internalFormat_ = GetTextureInternalFormat();

    /* Generate MIP-maps if enabled */
    if (initialImage != nullptr && MustGenerateMipsOnCreate(textureDesc))
        GLMipGenerator::Get().GenerateMips(textureDesc.type);
}

void GLTexture::AllocRenderbufferStorage(const TextureDescriptor& textureDesc)
{
    /* Allocate renderbuffer storage */
    GLRenderbuffer::AllocStorage(
        GetID(),
        GLTypes::Map(textureDesc.format),
        static_cast<GLsizei>(textureDesc.extent.width),
        static_cast<GLsizei>(textureDesc.extent.height),
        static_cast<GLsizei>(textureDesc.samples)
    );

    /* Store internal GL format */
    internalFormat_ = GetRenderbufferInternalFormat();
}

// Binds the specified GL texture temporarily. Only used to gather texture information, not to bind texture for the graphics or compute pipeline.
static void BindGLTextureNonPersistent(const GLTexture& textureGL)
{
    GLStateManager::Get().BindTexture(GLStateManager::GetTextureTarget(textureGL.GetType()), textureGL.GetID());
}

GLenum GLTexture::GetTextureInternalFormat() const
{
    /* Bind texture and query attributes */
    GLint format = 0;
    BindGLTextureNonPersistent(*this);
    GLProfile::GetTexParameterInternalFormat(GetGLTexLevelTarget(), &format);
    return static_cast<GLenum>(format);
}

GLenum GLTexture::GetRenderbufferInternalFormat() const
{
    /* Bind renderbuffer and query qttributes */
    GLint format = 0;
    GLStateManager::Get().BindRenderbuffer(GetID());
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT, &format);
    return static_cast<GLenum>(format);
}

void GLTexture::GetTextureParams(GLint* extent, GLint* samples) const
{
    #ifdef LLGL_GLEXT_GET_TEX_LEVEL_PARAMETER

    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        /* Query texture attributes directly using DSA */
        if (extent != nullptr)
        {
            glGetTextureLevelParameteriv(id_, 0, GL_TEXTURE_WIDTH,  &extent[0]);
            glGetTextureLevelParameteriv(id_, 0, GL_TEXTURE_HEIGHT, &extent[1]);
            glGetTextureLevelParameteriv(id_, 0, GL_TEXTURE_DEPTH,  &extent[2]);
        }

        if (samples != nullptr)
            glGetTextureLevelParameteriv(id_, 0, GL_TEXTURE_SAMPLES, samples);
    }
    else
    #endif
    {
        /* Push currently bound texture onto stack to restore it after query */
        GLStateManager::Get().PushBoundTexture(GLStateManager::GetTextureTarget(GetType()));
        {
            /* Bind texture and query attributes */
            BindGLTextureNonPersistent(*this);
            const GLenum target = GetGLTexLevelTarget();

            if (extent != nullptr)
            {
                glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH,  &extent[0]);
                glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &extent[1]);
                glGetTexLevelParameteriv(target, 0, GL_TEXTURE_DEPTH,  &extent[2]);
            }

            if (samples != nullptr)
                glGetTexLevelParameteriv(target, 0, GL_TEXTURE_SAMPLES, samples);
        }
        GLStateManager::Get().PopBoundTexture();
    }

    #else // LLGL_GLEXT_GET_TEX_LEVEL_PARAMETER

    if (extent != nullptr)
    {
        extent[0] = extent_[0];
        extent[1] = extent_[1];
        extent[2] = extent_[2];
    }

    if (samples != nullptr)
        samples[0] = samples_;

    #endif // /LLGL_GLEXT_GET_TEX_LEVEL_PARAMETER
}

void GLTexture::GetRenderbufferParams(GLint* extent, GLint* samples) const
{
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        /* Query texture attributes directly using DSA */
        if (extent != nullptr)
        {
            glGetNamedRenderbufferParameteriv(id_, GL_RENDERBUFFER_WIDTH,  &extent[0]);
            glGetNamedRenderbufferParameteriv(id_, GL_RENDERBUFFER_HEIGHT, &extent[1]);
            extent[2] = 1;
        }

        if (samples != nullptr)
            glGetNamedRenderbufferParameteriv(id_, GL_RENDERBUFFER_SAMPLES, samples);
    }
    else
    #endif
    {
        /* Push currently bound texture onto stack to restore it after query */
        GLStateManager::Get().PushBoundRenderbuffer();
        {
            /* Bind texture and query attributes */
            GLStateManager::Get().BindRenderbuffer(id_);

            if (extent != nullptr)
            {
                glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH,  &extent[0]);
                glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &extent[1]);
                extent[2] = 1;
            }

            if (samples != nullptr)
                glGetRenderbufferParameteriv(id_, GL_RENDERBUFFER_SAMPLES, samples);
        }
        GLStateManager::Get().PopBoundRenderbuffer();
    }
}

void GLTexture::GetTextureMipSize(GLint level, GLint (&texSize)[3]) const
{
    #ifdef LLGL_GLEXT_GET_TEX_LEVEL_PARAMETER

    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        /* Query texture attributes directly using DSA */
        glGetTextureLevelParameteriv(id_, level, GL_TEXTURE_WIDTH,  &texSize[0]);
        glGetTextureLevelParameteriv(id_, level, GL_TEXTURE_HEIGHT, &texSize[1]);
        glGetTextureLevelParameteriv(id_, level, GL_TEXTURE_DEPTH,  &texSize[2]);
    }
    else
    #endif
    {
        /* Push currently bound texture onto stack to restore it after query */
        GLStateManager::Get().PushBoundTexture(GLStateManager::GetTextureTarget(GetType()));
        {
            /* Bind texture and query attributes */
            BindGLTextureNonPersistent(*this);
            const GLenum target = GetGLTexLevelTarget();
            glGetTexLevelParameteriv(target, level, GL_TEXTURE_WIDTH,  &texSize[0]);
            glGetTexLevelParameteriv(target, level, GL_TEXTURE_HEIGHT, &texSize[1]);
            glGetTexLevelParameteriv(target, level, GL_TEXTURE_DEPTH,  &texSize[2]);
        }
        GLStateManager::Get().PopBoundTexture();
    }

    /* Adjust depth value for cube texture; the cube array texture on the other hand will contain a multiple of 6 already */
    if (GetType() == TextureType::TextureCube)
        texSize[2] *= 6;

    #else // LLGL_GLEXT_GET_TEX_LEVEL_PARAMETER

    /* Calculate MIP extent from texture size and type */
    auto GLIntToUInt32 = [](GLint x) -> std::uint32_t
    {
        return static_cast<std::uint32_t>(std::max(0, x));
    };

    auto extent = LLGL::GetMipExtent(
        GetType(),
        Extent3D
        {
            GLIntToUInt32(extent_[0]),
            GLIntToUInt32(extent_[1]),
            GLIntToUInt32(extent_[2])
        },
        GLIntToUInt32(level)
    );

    texSize[0] = extent.width;
    texSize[1] = extent.height;
    texSize[2] = extent.depth;

    #endif // /LLGL_GLEXT_GET_TEX_LEVEL_PARAMETER
}

void GLTexture::GetRenderbufferSize(GLint (&texSize)[3]) const
{
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        glGetNamedRenderbufferParameteriv(id_, GL_RENDERBUFFER_WIDTH, &texSize[0]);
        glGetNamedRenderbufferParameteriv(id_, GL_RENDERBUFFER_HEIGHT, &texSize[1]);
        texSize[2] = 1;
    }
    else
    #endif
    {
        /* Push currently bound texture onto stack to restore it after query */
        GLStateManager::Get().PushBoundRenderbuffer();
        {
            /* Bind texture and query attributes */
            GLStateManager::Get().BindRenderbuffer(id_);
            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH,  &texSize[0]);
            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &texSize[1]);
            texSize[2] = 1;
        }
        GLStateManager::Get().PopBoundRenderbuffer();
    }
}


} // /namespace LLGL



// ================================================================================
