/*
 * SwapChain.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

virtual bool IsPresentable(
    void
) const override final;

virtual void Present(
    void
) override final;

virtual std::uint32_t GetCurrentSwapIndex(
    void
) const override final;

virtual std::uint32_t GetNumSwapBuffers(
    void
) const override final;

virtual std::uint32_t GetSamples(
    void
) const override final;

virtual LLGL::Format GetColorFormat(
    void
) const override final;

virtual LLGL::Format GetDepthStencilFormat(
    void
) const override final;

virtual const LLGL::RenderPass* GetRenderPass(
    void
) const override;

virtual bool SetVsyncInterval(
    std::uint32_t vsyncInterval
) override final;



// ================================================================================
