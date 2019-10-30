/*
 * FillBufferByte4.metal
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

// Fill "clearValue" into the output buffer
kernel void CS(
    device uint*    outBuffer   [[buffer(0)]],
    constant uint&  clearValue  [[buffer(1)]],
    uint            threadID    [[thread_position_in_grid]])
{
    outBuffer[threadID] = clearValue;
}



// ================================================================================
