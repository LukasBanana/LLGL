/*
 * GLCommandQueue.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_COMMAND_QUEUE_H
#define LLGL_GL_COMMAND_QUEUE_H


#include <LLGL/CommandQueue.h>
#include <memory>


namespace LLGL
{


class GLStateManager;

class GLCommandQueue final : public CommandQueue
{

    public:

        #include <LLGL/Backend/CommandQueue.inl>

};


} // /namespace LLGL


#endif



// ================================================================================
