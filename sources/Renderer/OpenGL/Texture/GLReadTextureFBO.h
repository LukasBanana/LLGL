/*
 * GLReadTextureFBO.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
