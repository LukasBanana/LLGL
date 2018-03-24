/*
 * GLFence.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLFence.h"
#include "../Ext/GLExtensions.h"
#include "../../GLCommon/GLExtensionRegistry.h"


namespace LLGL
{


GLFence::~GLFence()
{
    Release();
}

void GLFence::Submit()
{
    if (HasExtension(GLExt::ARB_sync))
    {
        Release();
        sync_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
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


/*
 * ======= Private: =======
 */

void GLFence::Release()
{
    if (sync_)
    {
        glDeleteSync(sync_);
        sync_ = 0;
    }
}


} // /namespace LLGL



// ================================================================================
