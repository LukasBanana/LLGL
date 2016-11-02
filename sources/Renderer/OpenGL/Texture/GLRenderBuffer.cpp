/*
 * GLRenderbuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderbuffer.h"
#include "../Ext/GLExtensions.h"
#include "../RenderState/GLStateManager.h"


namespace LLGL
{


GLRenderbuffer::GLRenderbuffer()
{
    glGenRenderbuffers(1, &id_);
}

GLRenderbuffer::~GLRenderbuffer()
{
    glDeleteRenderbuffers(1, &id_);
}

void GLRenderbuffer::Bind() const
{
    GLStateManager::active->BindRenderbuffer(id_);
}

void GLRenderbuffer::Unbind() const
{
    GLStateManager::active->BindRenderbuffer(0);
}

void GLRenderbuffer::Recreate()
{
    /* Delete previous renderbuffer and create a new one */
    glDeleteRenderbuffers(1, &id_);
    glGenRenderbuffers(1, &id_);
}

void GLRenderbuffer::Storage(GLenum internalFormat, const Gs::Vector2i& size, GLsizei samples)
{
    if (samples > 0)
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalFormat, size.x, size.y);
    else
        glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, size.x, size.y);
}


} // /namespace LLGL



// ================================================================================
