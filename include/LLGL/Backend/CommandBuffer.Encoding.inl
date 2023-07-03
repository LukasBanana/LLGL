/*
 * CommandBuffer.Encoding.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Encoding ----- */

virtual void Begin(
    void
) override final;

virtual void End(
    void
) override final;

virtual void Execute(
    LLGL::CommandBuffer& deferredCommandBuffer
) override final;



// ================================================================================
