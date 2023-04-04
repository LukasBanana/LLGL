/*
 * CommandQueue.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_COMMAND_QUEUE_H
#define LLGL_COMMAND_QUEUE_H


#include <LLGL/RenderSystemChild.h>
#include <LLGL/ForwardDecls.h>
#include <cstdint>
#include <cstddef>


namespace LLGL
{


/**
\brief Command queue interface.
\remarks This class is used to submit one or more command buffers (aka. command lists)
into the command queue as well as CPU/GPU synchronization objects (aka. fences).
For immediate command buffers, the CommandQueue::Submit function has no effect.
*/
class LLGL_EXPORT CommandQueue : public RenderSystemChild
{

        LLGL_DECLARE_INTERFACE( InterfaceID::CommandQueue );

    public:

        /* ----- Command Buffers ----- */

        /**
        \brief Submits the specified command buffer to the command queue.
        \param[in] commandBuffer Specifies the command buffer that is to be submitted.
        If the command buffer was created with the CommandBufferFlags::ImmediateSubmit flag, this function has no effect.
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
        \brief Submits all command buffers in the specified array to the command queue at once.
        \see Submit(CommandBuffer&)
        */
        virtual void Submit(std::uint32_t numCommandBuffers, CommandBuffer* const * commandBuffers);
        #endif

        /* ----- Queries ----- */

        /**
        \brief Retrieves the result of the specified query objects.
        \param[in] queryHeap Specifies the query heap.
        \param[in] firstQuery Specifies the zero-based index of the first query within the heap.
        This must be in the half-open range [0, QueryHeapDescriptor::numQueries).
        \param[in] numQueries Specifies the number of queries to retrieve the result from.
        This must be less than or equal to (QueryHeapDescriptor::numQueries - firstQuery) and it must not be zero.
        \param[out] data Specifies the pointer to the output data. This must be a valid pointer to an array of \c numQueries entries.
        The array entries must have one of the following types:
        - std::uint32_t
        - std::uint64_t
        - QueryPipelineStatistics
        If the function return false, the content of this array is undefined.
        \param[in] dataSize Specifies the size (in bytes) of the output data.
        This must be equal to <code>numQueries * sizeof(T)</code> where \c T is the type of the query entries described above.
        \return True, if all results are available. Otherwise, the results are (partially) unavailable and the content of the output data is undefined.
        \remarks Here is a usage example:
        \code
        // Get results of 10 occlusion queries
        std::uint64_t occlusionQueryResults[10] = {};
        myCmdQueue->QueryResult(*myOcclusionQuery, 0, 10, occlusionQueryResults, sizeof(occlusionQueryResults));

        // Get result of a pipeline statistics query
        LLGL::QueryPipelineStatistics stats;
        myCmdQueue->QueryResult(*myPipelineStatsQuery, 0, 1, &stats, sizeof(stats));
        \endcode
        */
        virtual bool QueryResult(
            QueryHeap&      queryHeap,
            std::uint32_t   firstQuery,
            std::uint32_t   numQueries,
            void*           data,
            std::size_t     dataSize
        ) = 0;

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
