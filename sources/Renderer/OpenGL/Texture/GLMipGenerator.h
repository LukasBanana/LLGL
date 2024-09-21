/*
 * GLMipGenerator.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_MIP_GENERATOR_H
#define LLGL_GL_MIP_GENERATOR_H


#include <LLGL/TextureFlags.h>
#include <cstdint>
#include "GLFramebuffer.h"
#include "../OpenGL.h"


namespace LLGL
{


class GLTexture;
class GLStateManager;

class GLMipGenerator
{

    public:

        // Returns the instance of this singleton.
        static GLMipGenerator& Get();

    public:

        GLMipGenerator(const GLMipGenerator&) = delete;
        GLMipGenerator& operator = (const GLMipGenerator&) = delete;

        GLMipGenerator(GLMipGenerator&&) = delete;
        GLMipGenerator& operator = (GLMipGenerator&&) = delete;

        // Releases the resource for this singleton class.
        void Clear();

        // Generates the entire MIP-map chain for the currently bound OpenGL texture.
        void GenerateMips(const TextureType type);

        // Generates the entire MIP-map chain for the specified OpenGL texture.
        void GenerateMipsForTexture(GLStateManager& stateMngr, GLTexture& textureGL);

        // Generates the specified range of MIP-maps for the specified OpenGL texture.
        void GenerateMipsRangeForTexture(
            GLStateManager& stateMngr,
            GLTexture&      textureGL,
            std::uint32_t   baseMipLevel,
            std::uint32_t   numMipLevels,
            std::uint32_t   baseArrayLayer = 0,
            std::uint32_t   numArrayLayers = 1
        );

    private:

        GLMipGenerator() = default;

        void GenerateMipsPrimary(GLStateManager& stateMngr, GLuint texID, const TextureType texType);

        void GenerateMipsRangeWithFBO(
            GLStateManager& stateMngr,
            GLTexture&      textureGL,
            const Extent3D& extent,
            GLint           baseMipLevel,
            GLint           numMipLevels,
            GLint           baseArrayLayer,
            GLint           numArrayLayers
        );

        #if LLGL_GLEXT_TEXTURE_VIEW
        void GenerateMipsRangeWithTextureView(
            GLStateManager& stateMngr,
            GLTexture&      textureGL,
            GLuint          baseMipLevel,
            GLuint          numMipLevels,
            GLuint          baseArrayLayer,
            GLuint          numArrayLayers
        );
        #endif // /LLGL_GLEXT_TEXTURE_VIEW

    private:

        GLFramebufferPair mipGenerationFBOPair_;

};


} // /namespace LLGL


#endif



// ================================================================================
