/*
 * GLRenderbuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderbuffer.h"
#include "../RenderState/GLStateManager.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"


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
    GLStateManager::Get().DeleteRenderbuffer(id_);
}

static void GLRenderbufferStorage(GLuint id, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei samples)
{
    /* Bind renderbuffer and define storage with or without multi-sampling */
    GLStateManager::Get().BindRenderbuffer(id);
    if (samples > 1)
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalFormat, width, height);
    else
        glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
}

#if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT

static void GLNamedRenderbufferStorage(GLuint id, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei samples)
{
    /* Define storage of named renderbuffer with or without multi-sampling */
    if (samples > 1)
        glNamedRenderbufferStorageMultisample(id, samples, internalFormat, width, height);
    else
        glNamedRenderbufferStorage(id, internalFormat, width, height);
}

#endif // /GL_ARB_direct_state_access

void GLRenderbuffer::BindAndAllocStorage(GLenum internalFormat, GLsizei width, GLsizei height, GLsizei samples)
{
    GLRenderbufferStorage(id_, internalFormat, width, height, samples);
}

void GLRenderbuffer::AllocStorage(GLuint id, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei samples)
{
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        /* Define storage of named renderbuffer directly */
        GLNamedRenderbufferStorage(id, internalFormat, width, height, samples);
    }
    else
    #endif
    {
        /* Bind and define storage of renderbuffer */
        GLRenderbufferStorage(id, internalFormat, width, height, samples);
    }
}


} // /namespace LLGL



// ================================================================================
