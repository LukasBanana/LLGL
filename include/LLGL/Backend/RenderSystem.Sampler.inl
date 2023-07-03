/*
 * RenderSystem.Sampler.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Samplers ---- */

virtual LLGL::Sampler* CreateSampler(
    const LLGL::SamplerDescriptor&  samplerDesc
) override final;

virtual void Release(
    LLGL::Sampler&                  sampler
) override final;



// ================================================================================
