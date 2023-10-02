/*
 * DbgCommandQueue.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DBG_COMMAND_QUEUE_H
#define LLGL_DBG_COMMAND_QUEUE_H


#include <LLGL/CommandQueue.h>
#include <LLGL/RenderingDebugger.h>


namespace LLGL
{


class DbgQueryHeap;

class DbgCommandQueue final : public CommandQueue
{

    public:

        #include <LLGL/Backend/CommandQueue.inl>

    public:

        DbgCommandQueue(CommandQueue& instance, FrameProfile& profile, RenderingDebugger* debugger);

    public:

        CommandQueue& instance;

    private:

        void ValidateQueryResult(
            DbgQueryHeap&   queryHeap,
            std::uint32_t   firstQuery,
            std::uint32_t   numQueries,
            void*           data,
            std::size_t     dataSize
        );

    private:

        RenderingDebugger*  debugger_ = nullptr;
        FrameProfile&       profile_;

};


} // /namespace LLGL


#endif



// ================================================================================
