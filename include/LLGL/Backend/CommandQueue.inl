/*
 * CommandQueue.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Command Buffers ----- */

virtual void Submit(
    LLGL::CommandBuffer&    commandBuffer
) override final;

/* ----- Queries ----- */

virtual bool QueryResult(
    LLGL::QueryHeap&        queryHeap,
    std::uint32_t           firstQuery,
    std::uint32_t           numQueries,
    void*                   data,
    std::size_t             dataSize
) override final;

/* ----- Fences ----- */

virtual void Submit(
    LLGL::Fence&            fence
) override final;

virtual bool WaitFence(
    LLGL::Fence&            fence,
    std::uint64_t           timeout
) override final;

virtual void WaitIdle(
    void
) override final;



// ================================================================================
