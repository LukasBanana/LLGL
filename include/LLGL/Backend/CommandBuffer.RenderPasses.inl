/*
 * CommandBuffer.RenderPasses.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Render Passes ----- */

virtual void BeginRenderPass(
    RenderTarget&           renderTarget,
    const RenderPass*       renderPass      = nullptr,
    std::uint32_t           numClearValues  = 0,
    const ClearValue*       clearValues     = nullptr,
    std::uint32_t           swapBufferIndex = Constants::currentSwapIndex
) override final;

virtual void EndRenderPass(
    void
) override final;

virtual void Clear(
    long                    flags,
    const ClearValue&       clearValue      = {}
) override final;

virtual void ClearAttachments(
    std::uint32_t           numAttachments,
    const AttachmentClear*  attachments
) override final;



// ================================================================================
