/*
 * GLRenderBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderBuffer.h"
#include "../GLExtensions.h"
#include "../RenderState/GLStateManager.h"


namespace LLGL
{


GLRenderBuffer::GLRenderBuffer()
{
    glGenRenderbuffers(1, &id_);
}

GLRenderBuffer::~GLRenderBuffer()
{
    glDeleteRenderbuffers(1, &id_);
}

void GLRenderBuffer::Bind() const
{
    GLStateManager::active->BindRenderBuffer(id_);
}

void GLRenderBuffer::Unbind() const
{
    GLStateManager::active->BindRenderBuffer(0);
}

void GLRenderBuffer::Recreate()
{
    /* Delete previous renderbuffer and create a new one */
    glDeleteRenderbuffers(1, &id_);
    glGenRenderbuffers(1, &id_);
}

void GLRenderBuffer::Storage(GLenum internalFormat, GLsizei width, GLsizei height, GLsizei samples)
{
    if (samples > 0)
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalFormat, width, height);
    else
        glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
}


} // /namespace LLGL



// ================================================================================
