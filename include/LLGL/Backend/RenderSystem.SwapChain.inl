/*
 * RenderSystem.SwapChain.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Swap-chain ----- */

virtual LLGL::SwapChain* CreateSwapChain(
    const LLGL::SwapChainDescriptor&        swapChainDesc,
    const std::shared_ptr<LLGL::Surface>&   surface         = {}
) override final;

virtual void Release(
    LLGL::SwapChain&                        swapChain
) override final;



// ================================================================================
