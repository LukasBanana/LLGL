/*
 * RenderSystem.RenderPass.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Render Passes ----- */

virtual LLGL::RenderPass* CreateRenderPass(
    const LLGL::RenderPassDescriptor&   renderPassDesc
) override final;

virtual void Release(
    LLGL::RenderPass&                   renderPass
) override final;



// ================================================================================
