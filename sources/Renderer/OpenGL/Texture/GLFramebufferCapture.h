/*
 * GLFramebufferCapture.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_FRAMEBUFFER_CAPTURE_H
#define LLGL_GL_FRAMEBUFFER_CAPTURE_H


#include <LLGL/TextureFlags.h>
#include <cstdint>
#include "GLFramebuffer.h"
#include "../OpenGL.h"


namespace LLGL
{


class GLTexture;
class GLStateManager;

struct GLIntermediateTexture
{
    ~GLIntermediateTexture();

    void CreateTexture();
    void ReleaseTexture();

    GLuint texID = 0;
};

class GLFramebufferCapture
{

    public:

        // Returns the instance of this singleton.
        static GLFramebufferCapture& Get();

    public:

        GLFramebufferCapture(const GLFramebufferCapture&) = delete;
        GLFramebufferCapture& operator = (const GLFramebufferCapture&) = delete;

        GLFramebufferCapture(GLFramebufferCapture&&) = delete;
        GLFramebufferCapture& operator = (GLFramebufferCapture&&) = delete;

        // Releases the resource for this singleton class.
        void Clear();

        // Captures the current framebuffer and blits it into the specified texture.
        void CaptureFramebuffer(
            GLStateManager& stateMngr,
            GLTexture&      textureGL,
            GLint           dstLevel,
            const Offset3D& dstOffset,
            const Offset2D& srcOffset,
            const Extent2D& extent
        );

    private:

        GLFramebufferCapture() = default;

    private:

        GLIntermediateTexture   intermediateTex_;
        GLFramebufferPair       blitTextureFBOPair_;

};


} // /namespace LLGL


#endif



// ================================================================================
