/*
 * CommandBuffer.Compute.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Compute ----- */

virtual void Dispatch(
    std::uint32_t   numWorkGroupsX,
    std::uint32_t   numWorkGroupsY,
    std::uint32_t   numWorkGroupsZ
) override final;

virtual void DispatchIndirect(
    LLGL::Buffer&   buffer,
    std::uint64_t   offset
) override final;



// ================================================================================
