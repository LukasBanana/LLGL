/*
 * GLReadTextureFBO.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_READ_TEXTURE_FBO_H
#define LLGL_GL_READ_TEXTURE_FBO_H


#include "GLFramebuffer.h"


namespace LLGL
{


struct TextureLocation;
class GLTexture;

// Wrapper class for GL framebuffer objects (FBOs) of type GL_READ_FRAMEBUFFER used for texture read operations.
class GLReadTextureFBO
{

    public:

        GLReadTextureFBO();
        ~GLReadTextureFBO();

        void Attach(GLTexture& texture, GLint mipLevel, const Offset3D& offset);

        inline GLuint GetFBO() const
        {
            return fbo_.GetID();
        }

    private:

        GLFramebuffer fbo_;

};


} // /namespace LLGL


#endif



// ================================================================================
