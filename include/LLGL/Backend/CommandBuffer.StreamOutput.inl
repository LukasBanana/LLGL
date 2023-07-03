/*
 * CommandBuffer.StreamOutput.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Stream Output ------ */

virtual void BeginStreamOutput(
    std::uint32_t           numBuffers,
    LLGL::Buffer* const *   buffers
) override final;

virtual void EndStreamOutput(
    void
) override final;



// ================================================================================
