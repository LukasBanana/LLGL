/*
 * CommandBuffer.ViewportsAndScissors.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Viewport and Scissor ----- */

virtual void SetViewport(
    const LLGL::Viewport&   viewport
) override final;

virtual void SetViewports(
    std::uint32_t           numViewports,
    const LLGL::Viewport*   viewports
) override final;

virtual void SetScissor(
    const LLGL::Scissor&    scissor
) override final;

virtual void SetScissors(
    std::uint32_t           numScissors,
    const LLGL::Scissor*    scissors
) override final;



// ================================================================================
