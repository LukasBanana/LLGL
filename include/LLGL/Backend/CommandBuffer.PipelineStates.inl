/*
 * CommandBuffer.PipelineStates.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Pipeline States ----- */

virtual void SetPipelineState(
    LLGL::PipelineState&    pipelineState
) override final;

virtual void SetBlendFactor(
    const float             color[4]
) override final;

virtual void SetStencilReference(
    std::uint32_t           reference,
    const LLGL::StencilFace stencilFace = LLGL::StencilFace::FrontAndBack
) override final;

virtual void SetUniforms(
    std::uint32_t           first,
    const void*             data,
    std::uint16_t           dataSize
) override final;



// ================================================================================
