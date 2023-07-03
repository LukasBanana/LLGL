/*
 * RenderSystem.PipelineLayouts.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Pipeline Layouts ----- */

virtual LLGL::PipelineLayout* CreatePipelineLayout(
    const LLGL::PipelineLayoutDescriptor&   pipelineLayoutDesc
) override final;

virtual void Release(
    LLGL::PipelineLayout&                   pipelineLayout
) override final;



// ================================================================================
