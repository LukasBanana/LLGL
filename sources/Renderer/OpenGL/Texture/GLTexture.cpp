/*
 * GLTexture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLTexture.h"
#include "GLRenderbuffer.h"
#include "GLReadTextureFBO.h"
#include "GLMipGenerator.h"
#include "../GLObjectUtils.h"
#include "../RenderState/GLStateManager.h"
#include "../../GLCommon/GLTypes.h"
#include "../../GLCommon/GLExtensionRegistry.h"
#include "../../GLCommon/Texture/GLTexImage.h"
#include "../../GLCommon/Texture/GLTexSubImage.h"
#include "../../GLCommon/Texture/GLTextureSubImage.h"
#include "../Ext/GLExtensions.h"
#include "../../TextureUtils.h"


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
static GLSwizzleFormat MapSwizzleFormat(const Format format)
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
    swizzleFormat_  { MapSwizzleFormat(desc.format)            }
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
}

GLTexture::~GLTexture()
{
    if (IsRenderbuffer())
    {
        glDeleteRenderbuffers(1, &id_);
        GLStateManager::Get().NotifyRenderbufferRelease(id_);
    }
    else
    {
        glDeleteTextures(1, &id_);
        GLStateManager::Get().NotifyTextureRelease(id_, GLStateManager::GetTextureTarget(GetType()));
    }
}

void GLTexture::SetName(const char* name)
{
    if (IsRenderbuffer())
        GLSetObjectLabel(GL_RENDERBUFFER, GetID(), name);
    else
        GLSetObjectLabel(GL_TEXTURE, GetID(), name);
}

static GLenum GLGetTextureParamTarget(const TextureType type)
{
    switch (type)
    {
        case TextureType::Texture1D:        return GL_TEXTURE_1D;
        case TextureType::Texture2D:        return GL_TEXTURE_2D;
        case TextureType::Texture3D:        return GL_TEXTURE_3D;
        case TextureType::TextureCube:      return GL_TEXTURE_CUBE_MAP_POSITIVE_X; // use first cube face instead of texture type
        case TextureType::Texture1DArray:   return GL_TEXTURE_1D_ARRAY;
        case TextureType::Texture2DArray:   return GL_TEXTURE_2D_ARRAY;
        case TextureType::TextureCubeArray: return GL_TEXTURE_CUBE_MAP_POSITIVE_X; // use first cube face instead of texture type
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
    texDesc.bindFlags   = 0;
    texDesc.mipLevels   = static_cast<std::uint32_t>(GetNumMipLevels());

    /* Query hardware texture format and size */
    GLint extent[3] = {}, samples = 1;
    if (IsRenderbuffer())
        GetRenderbufferParams(extent, &samples);
    else
        GetTextureParams(extent, &samples);

    /*
    Translate data from OpenGL to LLGL.
    Note: for cube textures, depth extent can also be copied directly without transformation (no need to multiply by 6).
    */
    texDesc.format              = GetFormat();

    texDesc.extent.width        = static_cast<std::uint32_t>(extent[0]);
    texDesc.extent.height       = static_cast<std::uint32_t>(extent[1]);

    if (GetType() == TextureType::Texture3D)
        texDesc.extent.depth    = static_cast<std::uint32_t>(extent[2]);
    else
        texDesc.arrayLayers     = static_cast<std::uint32_t>(extent[2]);

    texDesc.samples             = static_cast<std::uint32_t>(samples);

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
    const auto format = GLTypes::UnmapFormat(GetGLInternalFormat());
    return MapGLSwizzleFormat(format, swizzleFormat_);
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

void GLTexture::BindAndAllocStorage(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc)
{
    /* Allocate texture or renderbuffer storage */
    if (IsRenderbuffer())
        AllocRenderbufferStorage(textureDesc);
    else
        AllocTextureStorage(textureDesc, imageDesc);

    /* Query and store internal format */
    QueryInternalFormat();
}

static TextureSwizzleRGBA GetTextureSwizzlePermutationBGRA(const TextureSwizzleRGBA& swizzle)
{
    TextureSwizzleRGBA permutation;
    {
        permutation.r = swizzle.b;
        permutation.g = swizzle.g;
        permutation.b = swizzle.r;
        permutation.a = swizzle.a;
    }
    return permutation;
}

// Maps the TextureSwizzleRGBA::a component to a different value for the "Alpha" swizzle format
static TextureSwizzle GetTextureSwizzlePermutationAlphaComponent(const TextureSwizzle swizzleAlpha)
{
    switch (swizzleAlpha)
    {
        case TextureSwizzle::Alpha: return TextureSwizzle::Red;     // Only alpha component can be mapped to another component
        case TextureSwizzle::Zero:  return TextureSwizzle::Zero;    // Zero is allowed as fixed value
        case TextureSwizzle::One:   return TextureSwizzle::One;     // One is allowed as fixed value
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

void GLTexture::InitializeTextureSwizzle(const TextureSwizzleRGBA& swizzle, bool ignoreIdentitySwizzle)
{
    /* Ignore initialization if default values can be used */
    if (swizzleFormat_ == GLSwizzleFormat::RGBA && ignoreIdentitySwizzle)
        return;

    /* Map swizzle parameters according for permutation format */
    const auto target = GetGLTexTarget();
    switch (swizzleFormat_)
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

#ifdef GL_ARB_copy_image

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
    glCopyImageSubData(
        srcTexture.GetID(),
        GLTypes::Map(srcTexture.GetType()),
        srcLevel,
        srcOffset.x,
        srcOffset.y,
        srcOffset.z,
        dstTexture.GetID(),
        GLTypes::Map(dstTexture.GetType()),
        dstLevel,
        dstOffset.x,
        dstOffset.y,
        dstOffset.z,
        static_cast<GLsizei>(extent.width),
        static_cast<GLsizei>(extent.height),
        static_cast<GLsizei>(extent.depth)
    );
}

#endif // /GL_ARB_copy_image

static void GLCopyTexSubImage(
    GLTexture&      dstTexture,
    GLint           dstLevel,
    const Offset3D& dstOffset,
    GLTexture&      srcTexture,
    GLint           srcLevel,
    Offset3D        srcOffset,
    const Extent3D& extent)
{
    const TextureType       type        = dstTexture.GetType();
    const GLTextureTarget   target      = GLStateManager::GetTextureTarget(type);
    const GLenum            targetGL    = GLTypes::Map(type);

    /* Store bound texture and framebuffer */
    GLStateManager::Get().PushBoundTexture(target);
    GLStateManager::Get().PushBoundFramebuffer(GLFramebufferTarget::READ_FRAMEBUFFER);
    {
        /* Create temporary FBO for source texture to read from GL_READ_FRAMEBUFFER in copy texture operator */
        GLReadTextureFBO readFBO;
        GLStateManager::Get().BindTexture(target, dstTexture.GetID());

        switch (type)
        {
            case TextureType::Texture1D:
            {
                readFBO.Attach(srcTexture, srcLevel, srcOffset);
                glCopyTexSubImage1D(
                    targetGL,
                    dstLevel,
                    dstOffset.x,
                    srcOffset.x,
                    0,
                    static_cast<GLsizei>(extent.width)
                );
            }
            break;

            case TextureType::Texture1DArray:
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

            case TextureType::Texture3D:
            case TextureType::Texture2DArray:
            case TextureType::Texture2DMSArray:
            case TextureType::TextureCube:
            case TextureType::TextureCubeArray:
            {
                for (decltype(extent.depth) i = 0; i < extent.depth; ++i)
                {
                    readFBO.Attach(srcTexture, srcLevel, srcOffset);
                    glCopyTexSubImage3D(
                        targetGL,
                        dstLevel,
                        dstOffset.x,
                        dstOffset.y,
                        dstOffset.z + i,
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
    if (!IsRenderbuffer())
    {
        //TODO
    }
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
    const SrcImageDescriptor imageDesc
    {
        formatAttribs.format,
        formatAttribs.dataType,
        reinterpret_cast<const void*>(offset),
        static_cast<std::size_t>(size)
    };

    /* Bind buffer to pixel transfer unpack buffer unit */
    GLStateManager::Get().BindBuffer(GLBufferTarget::PIXEL_UNPACK_BUFFER, bufferID);
    GLStateManager::Get().SetPixelStoreUnpack(rowLength, imageHeight, 1);
    {
        /* Write image sub data from currently bound unpack buffer */
        TextureSubImage(region, imageDesc);
    }
    GLStateManager::Get().SetPixelStoreUnpack(0, 0, 1);
    GLStateManager::Get().BindBuffer(GLBufferTarget::PIXEL_UNPACK_BUFFER, 0);
}

void GLTexture::TextureSubImage(const TextureRegion& region, const SrcImageDescriptor& imageDesc, bool restoreBoundTexture)
{
    if (!IsRenderbuffer())
    {
        #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
        if (HasExtension(GLExt::ARB_direct_state_access))
        {
            /* Transfer image data directly to GL texture */
            GLTextureSubImage(GetID(), GetType(), region, imageDesc, GetGLInternalFormat());
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
                    GLTexSubImage(GetType(), region, imageDesc, GetGLInternalFormat());
                }
                GLStateManager::Get().PopBoundTexture();
            }
            else
            {
                /* Bind texture and transfer image data to GL texture */
                GLStateManager::Get().BindTexture(target, GetID());
                GLTexSubImage(GetType(), region, imageDesc, GetGLInternalFormat());
            }
        }
    }
}

void GLTexture::TextureView(GLTexture& sharedTexture, const TextureViewDescriptor& textureViewDesc)
{
    if (!IsRenderbuffer())
    {
        #ifdef GL_ARB_texture_view
        if (HasExtension(GLExt::ARB_texture_view))
        {
            /* Initialize texture with texture-view description */
            glTextureView(
                GetID(),
                GLTypes::Map(textureViewDesc.type),
                sharedTexture.GetID(),
                GLTypes::Map(textureViewDesc.format),
                textureViewDesc.subresource.baseMipLevel,
                textureViewDesc.subresource.numMipLevels,
                textureViewDesc.subresource.baseArrayLayer,
                textureViewDesc.subresource.numArrayLayers
            );
        }
        #endif // /GL_ARB_texture_view
    }
}

GLsizei GLTexture::GetRegionMemoryFootprint(const Extent3D& extent, const TextureSubresource& subresource) const
{
    const auto numTexels = NumMipTexels(GetType(), extent, subresource);
    return static_cast<GLsizei>(GetMemoryFootprint(GetFormat(), numTexels));
}

GLenum GLTexture::GetGLTexTarget() const
{
    return GLTypes::Map(GetType());
}


/*
 * ======= Private: =======
 */

void GLTexture::AllocTextureStorage(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc)
{
    /* Bind texture */
    GLStateManager::Get().BindGLTexture(*this);

    /* Initialize texture parameters for the first time */
    auto target = GLTypes::Map(textureDesc.type);

    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GetGlTextureMinFilter(textureDesc));
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /* Configure texture swizzling if format is not supported */
    InitializeTextureSwizzle({}, true);

    /* Convert initial image data for texture swizzle formats */
    SrcImageDescriptor intermediateImageDesc;

    if (imageDesc != nullptr && GetSwizzleFormat() == GLSwizzleFormat::BGRA)
    {
        intermediateImageDesc = *imageDesc;
        intermediateImageDesc.format = MapSwizzleImageFormat(imageDesc->format);
        imageDesc = &intermediateImageDesc;
    }

    /* Build texture storage and upload image dataa */
    //GLStateManager::Get().BindBuffer(GLBufferTarget::PIXEL_UNPACK_BUFFER, 0);
    GLTexImage(textureDesc, imageDesc);

    /* Generate MIP-maps if enabled */
    if (imageDesc != nullptr && MustGenerateMipsOnCreate(textureDesc))
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
}

void GLTexture::QueryInternalFormat()
{
    GLint format = 0;
    {
        if (IsRenderbuffer())
            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT, &format);
        else
            glGetTexLevelParameteriv(GetGLTexTarget(), 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
    }
    internalFormat_ = static_cast<GLenum>(format);
}

void GLTexture::GetTextureParams(GLint* extent, GLint* samples) const
{
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
            GLStateManager::Get().BindGLTexture(*this);
            auto target = GLGetTextureParamTarget(GetType());

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
            GLStateManager::Get().BindGLTexture(*this);
            auto target = GLGetTextureParamTarget(GetType());
            glGetTexLevelParameteriv(target, level, GL_TEXTURE_WIDTH,  &texSize[0]);
            glGetTexLevelParameteriv(target, level, GL_TEXTURE_HEIGHT, &texSize[1]);
            glGetTexLevelParameteriv(target, level, GL_TEXTURE_DEPTH,  &texSize[2]);
        }
        GLStateManager::Get().PopBoundTexture();
    }

    /* Adjust depth value for cube texture to be uniform with D3D */
    if (IsCubeTexture(GetType()))
        texSize[2] *= 6;
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
