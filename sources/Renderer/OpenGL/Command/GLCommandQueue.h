/*
 * GLCommandQueue.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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

        GLCommandQueue(const std::shared_ptr<GLStateManager>& stateManager);

        /* ----- Command Buffers ----- */

        void Submit(CommandBuffer& commandBuffer) override;

        /* ----- Queries ----- */

        bool QueryResult(
            QueryHeap&      queryHeap,
            std::uint32_t   firstQuery,
            std::uint32_t   numQueries,
            void*           data,
            std::size_t     dataSize
        ) override;

        /* ----- Fences ----- */

        void Submit(Fence& fence) override;

        bool WaitFence(Fence& fence, std::uint64_t timeout) override;
        void WaitIdle() override;

    private:

        std::shared_ptr<GLStateManager> stateMngr_;

};


} // /namespace LLGL


#endif



// ================================================================================
