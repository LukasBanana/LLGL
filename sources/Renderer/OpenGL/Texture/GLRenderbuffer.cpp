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


GLRenderbuffer::GLRenderbuffer()
{
    glGenRenderbuffers(1, &id_);
}

GLRenderbuffer::~GLRenderbuffer()
{
    glDeleteRenderbuffers(1, &id_);
}

void GLRenderbuffer::Storage(GLenum internalFormat, GLsizei width, GLsizei height, GLsizei samples)
{
    /* Bind renderbuffer */
    GLStateManager::active->BindRenderbuffer(id_);

    /* Initialize renderbuffer storage */
    if (samples > 1)
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalFormat, width, height);
    else
        glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
}


} // /namespace LLGL



// ================================================================================
