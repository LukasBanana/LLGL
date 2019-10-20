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


// Predefined texture swizzles to emulate certain texture format
enum class GLSwizzleFormat
{
    RGBA,   // GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA
    BGRA,   // GL_BLUE, GL_GREEN, GL_RED, GL_ALPHA
    Alpha,  // GL_ZERO, GL_ZERO, GL_ZERO, GL_RED
};

// OpenGL texture class that manages a GL textures and renderbuffers (if the texture is only used as attachment but not for sampling).
class GLTexture final : public Texture
{

    public:

        void SetName(const char* name) override;

        Extent3D GetMipExtent(std::uint32_t mipLevel) const override;

        TextureDescriptor GetDesc() const override;

    public:

        GLTexture(const TextureDescriptor& desc);
        ~GLTexture();

        // Initialize the texture swizzle parameters; the texture must already be bound to an active texture layer.
        void InitializeTextureSwizzle(const TextureSwizzleRGBA& swizzle = {}, bool ignoreIdentitySwizzle = false);

        // Copies the specified source texture into this texture.
        void CopyImageSubData(
            GLint           dstLevel,
            const Offset3D& dstOffset,
            GLTexture&      srcTexture,
            GLint           srcLevel,
            const Offset3D& srcOffset,
            const Extent3D& extent
        );

        // Initializes this texture as a texture-view.
        void TextureView(GLTexture& sharedTexture, const TextureViewDescriptor& textureViewDesc);

        // Returns the GL_TEXTURE_INTERNAL_FORMAT parameter of this texture.
        GLenum GetInternalFormat() const;

        // Returns the hardware texture ID.
        inline GLuint GetID() const
        {
            return id_;
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

    private:

        void GetTextureParams(GLint* internalFormat, GLint* extent, GLint* samples) const;
        void GetRenderbufferParams(GLint* internalFormat, GLint* extent, GLint* samples) const;

        void GetTextureMipSize(GLint level, GLint (&texSize)[3]) const;
        void GetRenderbufferSize(GLint (&texSize)[3]) const;

    private:

        GLuint          id_             = 0;                        // GL object name for texture or renderbuffer
        GLsizei         numMipLevels_   = 1;
        bool            isRenderbuffer_ = false;
        GLSwizzleFormat swizzleFormat_  = GLSwizzleFormat::RGBA;    // Identity texture swizzle by default

};


} // /namespace LLGL


#endif



// ================================================================================
