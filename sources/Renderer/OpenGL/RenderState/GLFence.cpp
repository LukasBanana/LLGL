/*
 * GLFence.cpp
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLFence.h"
#include "../GLObjectUtils.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"


namespace LLGL
{


GLFence::~GLFence()
{
    /* Always call glDeleteSync, it will silently ignore a <sync> value of zero */
    glDeleteSync(sync_);
}

void GLFence::SetName(const char* name)
{
    #ifdef LLGL_DEBUG
    /* Only store name in fence object in debug mode, otherwise we want to keep fence objects as lightweight as possible */
    name_ = name;
    #endif
}

void GLFence::Submit()
{
    if (HasExtension(GLExt::ARB_sync))
    {
        #ifdef LLGL_DEBUG
        /* Re-assign debug name after new sync object is created */
        if (sync_)
            GLSetObjectPtrLabel(sync_, nullptr);
        #endif

        /* Generate new sync object */
        glDeleteSync(sync_);
        sync_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

        #ifdef LLGL_DEBUG
        /* Re-assign debug name after new sync object is created */
        if (!name_.empty())
            GLSetObjectPtrLabel(sync_, name_.c_str());
        #endif
    }
}

bool GLFence::Wait(GLuint64 timeout)
{
    if (HasExtension(GLExt::ARB_sync))
    {
        GLenum result = glClientWaitSync(sync_, GL_SYNC_FLUSH_COMMANDS_BIT, timeout);
        return (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED);
    }
    else
    {
        glFinish();
        return true;
    }
}


} // /namespace LLGL



// ================================================================================
