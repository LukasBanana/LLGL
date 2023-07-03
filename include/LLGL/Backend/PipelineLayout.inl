/*
 * PipelineLayout.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

virtual std::uint32_t GetNumHeapBindings(
    void
) const override final;

virtual std::uint32_t GetNumBindings(
    void
) const override final;

virtual std::uint32_t GetNumStaticSamplers(
    void
) const override final;

virtual std::uint32_t GetNumUniforms(
    void
) const override final;



// ================================================================================
