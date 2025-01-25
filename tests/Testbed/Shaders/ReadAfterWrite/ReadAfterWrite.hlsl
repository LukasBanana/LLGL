/*
 * ReadAfterWrite.hlsl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

RWBuffer<uint> buf1 : register(u1);

struct Entry
{
    uint a, b;
};

RWStructuredBuffer<Entry> buf2 : register(u2);

RWTexture1D<uint> tex1 : register(u3);
RWTexture2D<uint2> tex2 : register(u4);

uint readPos;
uint writePos;

[numthreads(1, 1, 1)]
void CSMain(uint id : SV_DispatchThreadID)
{
    // Propagate values from read position to write position
    buf1[writePos + id] = buf1[readPos + id];
    buf2[writePos + id] = buf2[readPos + id];
    tex1[writePos + id] = tex1[readPos + id];
    tex2[uint2(writePos + id, 0)] = tex2[uint2(readPos + id, 0)];
}
