/*
 * NullCommandQueue.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_NULL_COMMAND_QUEUE_H
#define LLGL_NULL_COMMAND_QUEUE_H


#include <LLGL/CommandQueue.h>


namespace LLGL
{


class NullCommandQueue final : public CommandQueue
{

    public:

        #include <LLGL/Backend/CommandQueue.inl>

};


} // /namespace LLGL


#endif



// ================================================================================
