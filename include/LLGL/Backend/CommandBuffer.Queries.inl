/*
 * CommandBuffer.Queries.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Queries ----- */

virtual void BeginQuery(
    QueryHeap&                  queryHeap,
    std::uint32_t               query
) override final;

virtual void EndQuery(
    QueryHeap&                  queryHeap,
    std::uint32_t               query
) override final;

virtual void BeginRenderCondition(
    QueryHeap&                  queryHeap,
    std::uint32_t               query,
    const RenderConditionMode   mode
) override final;

virtual void EndRenderCondition(
    void
) override final;



// ================================================================================
