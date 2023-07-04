/*
 * GLCommandExecutor.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_COMMAND_EXECUTOR_H
#define LLGL_GL_COMMAND_EXECUTOR_H


namespace LLGL
{


namespace OpenGL { struct NativeCommand; }

class GLStateManager;
class GLCommandBuffer;
class GLDeferredCommandBuffer;

/*
Executes all GL commands that have been recorded in the specified command buffer.
GL render states are tracked with the specified state manager.
*/
void ExecuteGLDeferredCommandBuffer(const GLDeferredCommandBuffer& cmdbuffer, GLStateManager& stateMngr);
void ExecuteGLCommandBuffer(const GLCommandBuffer& cmdbuffer, GLStateManager& stateMngr);

// Executes the specified native GL command.
void ExecuteNativeGLCommand(const OpenGL::NativeCommand& cmd, GLStateManager& stateMngr);


} // /namespace LLGL


#endif



// ================================================================================
