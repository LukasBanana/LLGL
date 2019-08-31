/*
 * GLCommandAssembler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
