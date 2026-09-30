/*
 * CommandBufferTier1.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Backend/CommandBuffer.inl>

/* ----- Variable Rate Shading (VRS) ----- */

virtual void SetShadingRate(
    LLGL::ShadingRate   shadingRate
) override final;

virtual void SetShadingRate(
    LLGL::ShadingRate   shadingRate,
    LLGL::ShadingRateOp combinerOpX,
    LLGL::ShadingRateOp combinerOpY
) override final;

/* ----- Mesh pipeline ----- */

virtual void DrawMesh(
    std::uint32_t       numWorkGroupsX,
    std::uint32_t       numWorkGroupsY,
    std::uint32_t       numWorkGroupsZ
) override final;

virtual void DrawMeshIndirect(
    LLGL::Buffer&       buffer,
    std::uint64_t       offset,
    std::uint32_t       numCommands,
    std::uint32_t       stride
) override final;

virtual void DrawMeshIndirect(
    LLGL::Buffer&       argumentsBuffer,
    std::uint64_t       argumentsOffset,
    LLGL::Buffer&       countBuffer,
    std::uint64_t       countOffset,
    std::uint32_t       maxNumCommands,
    std::uint32_t       stride
) override final;



// ================================================================================
