/*
 * RenderSystem.ResourceHeap.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Resource Heaps ----- */

virtual LLGL::ResourceHeap* CreateResourceHeap(
    const LLGL::ResourceHeapDescriptor&                     resourceHeapDesc,
    const LLGL::ArrayView<LLGL::ResourceViewDescriptor>&    initialResourceViews = {}
) override final;

virtual void Release(
    LLGL::ResourceHeap&                                     resourceHeap
) override final;

virtual std::uint32_t WriteResourceHeap(
    LLGL::ResourceHeap&                                     resourceHeap,
    std::uint32_t                                           firstDescriptor,
    const LLGL::ArrayView<LLGL::ResourceViewDescriptor>&    resourceViews
) override final;



// ================================================================================
