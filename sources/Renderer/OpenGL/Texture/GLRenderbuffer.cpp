/*
 * GLRenderbuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderbuffer.h"
#include "../Ext/GLExtensions.h"
#include "../RenderState/GLStateManager.h"


namespace LLGL
{


GLRenderbuffer::~GLRenderbuffer()
{
    DeleteRenderbuffer();
}

GLRenderbuffer::GLRenderbuffer(GLRenderbuffer&& rhs) :
    id_ { rhs.id_ }
{
    rhs.id_ = 0;
}

GLRenderbuffer& GLRenderbuffer::operator = (GLRenderbuffer&& rhs)
{
    if (id_ != rhs.id_)
    {
        DeleteRenderbuffer();
        id_ = rhs.id_;
        rhs.id_ = 0;
    }
    return *this;
}

void GLRenderbuffer::GenRenderbuffer()
{
    DeleteRenderbuffer();
    glGenRenderbuffers(1, &id_);
}

void GLRenderbuffer::DeleteRenderbuffer()
{
    if (id_ != 0)
    {
        glDeleteRenderbuffers(1, &id_);
        GLStateManager::Get().NotifyRenderbufferRelease(id_);
        id_ = 0;
    }
}

void GLRenderbuffer::Storage(GLenum internalFormat, GLsizei width, GLsizei height, GLsizei samples)
{
    /* Bind renderbuffer */
    GLStateManager::Get().BindRenderbuffer(id_);

    /* Initialize renderbuffer storage */
    if (samples > 1)
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalFormat, width, height);
    else
        glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
}


} // /namespace LLGL



// ================================================================================
