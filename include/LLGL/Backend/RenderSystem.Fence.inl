/*
 * RenderSystem.Fence.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Fences ----- */

virtual LLGL::Fence* CreateFence(
    void
) override final;

virtual void Release(
    LLGL::Fence& fence
) override final;



// ================================================================================
