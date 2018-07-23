/*
 * CommandQueue.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_COMMAND_QUEUE_H
#define LLGL_COMMAND_QUEUE_H


#include "RenderSystemChild.h"
#include <cstdint>


namespace LLGL
{


class CommandBuffer;
class Fence;

/**
\brief Command queue interface.
\remarks This class is mainly used for modern rendering APIs (such as Direct3D 12 and Vulkan)
to submit one ore more command buffers (or command lists) into the command queue.
For older rendering APIs (such as Direct3D 11 and OpenGL) submitting a command buffer has no effect.
It also provides the functionality to submit small sized objects such as fences into the command queue.
*/
class LLGL_EXPORT CommandQueue : public RenderSystemChild
{

    public:

        /* ----- Command Buffers ----- */

        /**
        \brief Submits the specified command buffer to the command queue.
        \remarks This must only be called with a command buffer that has already been encoded via the \c Begin and \c End functions:
        \code
        myCmdBuffer->Begin();
        // Encode/record command buffer ...
        myCmdBuffer->End();
        myCmdQueue->Submit(*myCmdBuffer);
        \endcode
        \see CommandBuffer::Begin
        \see CommandBuffer::End
        \see Submit(std::uint32_t, CommandBuffer* const *)
        */
        virtual void Submit(CommandBuffer& commandBuffer) = 0;

        #if 0
        /**
        \briefs Submits all command buffers in the specified array to the command queue at once.
        \see Submit(CommandBuffer&)
        */
        virtual void Submit(std::uint32_t numCommandBuffers, CommandBuffer* const * commandBuffers);
        #endif

        /* ----- Fences ----- */

        //! Submits the specified fence to the command queue for CPU/GPU synchronization.
        virtual void Submit(Fence& fence) = 0;

        /**
        \brief Blocks the CPU execution until the specified fence has been signaled.
        \param[in] fence Specifies the fence for which the CPU needs to wait to be signaled.
        \param[in] timeout Specifies the waiting timeout (in nanoseconds).
        \return True on success, or false if the fence has a timeout (in nanoseconds) or the device is lost.
        \remarks To wait for the completion of the entire GPU command queue, use 'WaitIdle'.
        \see WaitIdle
        */
        virtual bool WaitFence(Fence& fence, std::uint64_t timeout) = 0;

        /**
        \brief Blocks the CPU execution until the entire GPU command queue has been completed.
        \remarks To wait for a specific point in the command queue, use fences.
        Waiting for the queue to be become idle is equivalent to submitting a fence and waiting for that fence to be signaled:
        \code
        myCmdQueue->Submit(myFence);
        myCmdQueue->WaitFence(myFence, ~0);
        \endcode
        \see WaitFence
        */
        virtual void WaitIdle() = 0;

    protected:

        CommandQueue() = default;

};


} // /namespace LLGL


#endif



// ================================================================================
