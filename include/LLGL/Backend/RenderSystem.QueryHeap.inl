/*
 * RenderSystem.QueryHeap.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Query heaps ----- */

virtual LLGL::QueryHeap* CreateQueryHeap(
    const LLGL::QueryHeapDescriptor&    queryHeapDesc
) override final;

virtual void Release(
    LLGL::QueryHeap&                    queryHeap
) override final;



// ================================================================================
