/*
 * GLCommandAssembler.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_COMMAND_ASSEMBLER_H
#define LLGL_GL_COMMAND_ASSEMBLER_H

#ifdef LLGL_ENABLE_JIT_COMPILER


#include <memory>


namespace LLGL
{


class JITProgram;
class GLDeferredCommandBuffer;

std::unique_ptr<JITProgram> AssembleGLDeferredCommandBuffer(const GLDeferredCommandBuffer& cmdbuffer);


} // /namespace LLGL


#endif // /LLGL_ENABLE_JIT_COMPILER

#endif



// ================================================================================
