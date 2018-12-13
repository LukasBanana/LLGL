/*
 * GLCommandBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_COMMAND_BUFFER_H
#define LLGL_GL_COMMAND_BUFFER_H


#include <LLGL/CommandBufferExt.h>


namespace LLGL
{


class GLCommandBuffer : public CommandBufferExt
{

    public:

        virtual bool IsImmediateCmdBuffer() const = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
