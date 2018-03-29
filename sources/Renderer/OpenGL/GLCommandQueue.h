/*
 * GLCommandQueue.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_COMMAND_QUEUE_H
#define LLGL_GL_COMMAND_QUEUE_H


#include <LLGL/CommandQueue.h>


namespace LLGL
{


class GLCommandQueue : public CommandQueue
{

    public:

        /* ----- Command queues ----- */

        void Submit(CommandBuffer& commandBuffer) override;

        /* ----- Fences ----- */

        void Submit(Fence& fence) override;

        bool WaitForFence(Fence& fence, std::uint64_t timeout) override;
        void WaitForFinish() override;

};


} // /namespace LLGL


#endif



// ================================================================================
