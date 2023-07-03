/*
 * RenderSystem.CommandBuffer.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Command buffers ----- */

virtual LLGL::CommandBuffer* CreateCommandBuffer(
    const LLGL::CommandBufferDescriptor&    commandBufferDesc = {}
) override final;

virtual void Release(
    LLGL::CommandBuffer&                    commandBuffer
) override final;



// ================================================================================
