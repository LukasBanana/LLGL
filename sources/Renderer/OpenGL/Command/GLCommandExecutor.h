/*
 * GLCommandExecutor.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_COMMAND_EXECUTOR_H
#define LLGL_GL_COMMAND_EXECUTOR_H


namespace LLGL
{


class GLStateManager;
class GLCommandBuffer;
class GLDeferredCommandBuffer;

void ExecuteGLDeferredCommandBuffer(const GLDeferredCommandBuffer& cmdbuffer, GLStateManager& stateMngr);
void ExecuteGLCommandBuffer(const GLCommandBuffer& cmdbuffer, GLStateManager& stateMngr);


} // /namespace LLGL


#endif



// ================================================================================
