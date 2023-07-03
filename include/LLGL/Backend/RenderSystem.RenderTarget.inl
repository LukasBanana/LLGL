/*
 * RenderSystem.RenderTarget.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Render Targets ----- */

virtual LLGL::RenderTarget* CreateRenderTarget(
    const LLGL::RenderTargetDescriptor& renderTargetDesc
) override final;

virtual void Release(
    LLGL::RenderTarget&                 renderTarget
) override final;



// ================================================================================
