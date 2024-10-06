/*
 * StreamOutputDrawArgs.hlsl
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

cbuffer StreamOutputFormat : register(b0)
{
    uint bufferStride;
}

RWByteAddressBuffer outIndirectDrawArgs : register(u0);

/* Compute kernel to generate indirect draw arguments from a stream-output fill buffer value */
[RootSignature(
    "RootFlags(0),"
    "RootConstants(b0, num32BitConstants = 1),"
    "UAV(u0)"
)]
[numthreads(1, 1, 1)]
void StreamOutputDrawArgsCS()
{
    /* Load stream-output fill buffer value (UINT64) */
    uint64_t streamOutputFillSize = outIndirectDrawArgs.Load<uint64_t>(0);

    /* Write out indirect draw arguments (D3D12_DRAW_ARGUMENTS) */
    outIndirectDrawArgs.Store4(0, uint4((uint)(streamOutputFillSize / bufferStride), 1, 0, 0));
}

