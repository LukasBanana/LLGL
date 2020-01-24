/*
 * GLTexture.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_TEXTURE_H
#define LLGL_GL_TEXTURE_H


#include <LLGL/Texture.h>
#include "../OpenGL.h"


namespace LLGL
{


struct SrcImageDescriptor;
struct DstImageDescriptor;
struct TextureViewDescriptor;

// Predefined texture swizzles to emulate certain texture format
enum class GLSwizzleFormat
{
    RGBA,   // GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA
    BGRA,   // GL_BLUE, GL_GREEN, GL_RED, GL_ALPHA
    Alpha,  // GL_ZERO, GL_ZERO, GL_ZERO, GL_RED
};

GLSwizzleFormat MapGLSwizzleFormat(const Format format);

// OpenGL texture class that manages a GL textures and renderbuffers (if the texture is only used as attachment but not for sampling).
class GLTexture final : public Texture
{

    public:

        void SetName(const char* name) override;

        Extent3D GetMipExtent(std::uint32_t mipLevel) const override;

        TextureDescriptor GetDesc() const override;

        Format GetFormat() const override;

    public:

        GLTexture(const TextureDescriptor& desc);
        ~GLTexture();

        // Initializes the texture storage with an optional image data; the texture will be bound to the current active texture unit.
        void BindAndAllocStorage(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc = nullptr);

        // Copies the specified source texture into this texture.
        void CopyImageSubData(
            GLint           dstLevel,
            const Offset3D& dstOffset,
            GLTexture&      srcTexture,
            GLint           srcLevel,
            const Offset3D& srcOffset,
            const Extent3D& extent
        );

        // Reads pixels from this texture and stores it to the specified buffer (glGetTextureSubImage with GL_PIXEL_PACK_BUFFER).
        void CopyImageToBuffer(
            const TextureRegion&    region,
            GLuint                  bufferID,
            GLintptr                offset,
            GLsizei                 size,
            GLint                   rowLength   = 0,
            GLint                   imageHeight = 0
        );

        // Writes pixesl to this texture and reads it from the specified buffer (glTexSubImage* with GL_PIXEL_UNPACK_BUFFER).
        void CopyImageFromBuffer(
            const TextureRegion&    region,
            GLuint                  bufferID,
            GLintptr                offset,
            GLsizei                 size,
            GLint                   rowLength   = 0,
            GLint                   imageHeight = 0
        );

        // Writes the specified image data to a subregion of this texture.
        void TextureSubImage(const TextureRegion& region, const SrcImageDescriptor& imageDesc, bool restoreBoundTexture = true);

        // Reads the specified image data from a subregion of this texture.
        void GetTextureSubImage(const TextureRegion& region, const DstImageDescriptor& imageDesc, bool restoreBoundTexture = true);

        // Returns the GL_TEXTURE_TARGET parameter of this texture.
        GLenum GetGLTexTarget() const;

        // Returns the hardware texture ID.
        inline GLuint GetID() const
        {
            return id_;
        }

        // Returns the GL_TEXTURE_INTERNAL_FORMAT parameter of this texture.
        inline GLenum GetGLInternalFormat() const
        {
            return internalFormat_;
        }

        // Returns the number of MIP-map levels.
        inline GLsizei GetNumMipLevels() const
        {
            return numMipLevels_;
        }

        // Returns true if this object managges a GL renderbuffer instead of a texture.
        inline bool IsRenderbuffer() const
        {
            return isRenderbuffer_;
        }

        // Returns the texture swizzle format.
        inline GLSwizzleFormat GetSwizzleFormat() const
        {
            return swizzleFormat_;
        }

    public:

        // Initialize the texture swizzle parameters; the texture must already be bound to an active texture layer.
        static void TexParameterSwizzle(
            const TextureType           type,
            const Format                format,
            const TextureSwizzleRGBA&   swizzle,
            bool                        ignoreIdentitySwizzle   = false
        );

    private:

        void AllocTextureStorage(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc);
        void AllocRenderbufferStorage(const TextureDescriptor& textureDesc);

        void QueryInternalFormat();

        void GetTextureParams(GLint* extent, GLint* samples) const;
        void GetRenderbufferParams(GLint* extent, GLint* samples) const;

        void GetTextureMipSize(GLint level, GLint (&texSize)[3]) const;
        void GetRenderbufferSize(GLint (&texSize)[3]) const;

    private:

        GLuint          id_             = 0;                        // GL object name for texture or renderbuffer
        GLenum          internalFormat_ = 0;
        GLsizei         numMipLevels_   = 1;
        bool            isRenderbuffer_ = false;
        GLSwizzleFormat swizzleFormat_  = GLSwizzleFormat::RGBA;    // Identity texture swizzle by default

        #ifdef LLGL_OPENGLES3
        GLint           extent_[3]      = {};
        GLint           samples_        = 1;
        #endif

};


} // /namespace LLGL


#endif



// ================================================================================
