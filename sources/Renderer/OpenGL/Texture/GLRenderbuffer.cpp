/*
 * GLRenderbuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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

#if LLGL_GLEXT_DIRECT_STATE_ACCESS

static void GLNamedRenderbufferStorage(GLuint id, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei samples)
{
    /* Define storage of named renderbuffer with or without multi-sampling */
    if (samples > 1)
        glNamedRenderbufferStorageMultisample(id, samples, internalFormat, width, height);
    else
        glNamedRenderbufferStorage(id, internalFormat, width, height);
}

#endif // /LLGL_GLEXT_DIRECT_STATE_ACCESS

void GLRenderbuffer::BindAndAllocStorage(GLenum internalFormat, GLsizei width, GLsizei height, GLsizei samples)
{
    GLRenderbufferStorage(id_, internalFormat, width, height, samples);
}

void GLRenderbuffer::AllocStorage(GLuint id, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei samples)
{
    #if LLGL_GLEXT_DIRECT_STATE_ACCESS
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        /* Define storage of named renderbuffer directly */
        GLNamedRenderbufferStorage(id, internalFormat, width, height, samples);
    }
    else
    #endif // /LLGL_GLEXT_DIRECT_STATE_ACCESS
    {
        /* Bind and define storage of renderbuffer */
        GLRenderbufferStorage(id, internalFormat, width, height, samples);
    }
}


} // /namespace LLGL



// ================================================================================
