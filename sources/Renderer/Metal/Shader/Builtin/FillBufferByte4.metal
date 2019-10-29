/*
 * FillBufferByte4.metal
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

// Fill "clearValue" into the output buffer
kernel void CS(
    device uint*    outBuffer   [[buffer(0)]],
    uint            threadID    [[thread_position_in_grid]])
{
    outBuffer[threadID] = outBuffer[0];
}



// ================================================================================
