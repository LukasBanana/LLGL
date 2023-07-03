/*
 * RenderTarget.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

virtual LLGL::Extent2D GetResolution(
    void
) const override final;

virtual std::uint32_t GetSamples(
    void
) const override final;

virtual std::uint32_t GetNumColorAttachments(
    void
) const override final;

virtual bool HasDepthAttachment(
    void
) const override final;

virtual bool HasStencilAttachment(
    void
) const override final;

virtual const LLGL::RenderPass* GetRenderPass(
    void
) const override final;



// ================================================================================
