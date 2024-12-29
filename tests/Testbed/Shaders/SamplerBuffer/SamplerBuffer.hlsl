/*
 * SamplerBuffer.hlsl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

cbuffer Config : register(b4)
{
    int3 multipliers;
}

Buffer<int> inTypedBuffer : register(t0);
RWBuffer<int> outTypedBuffer : register(u1);

struct Entry
{
    int a, b;
};

StructuredBuffer<Entry> inStructBuffer : register(t2);
RWStructuredBuffer<Entry> outStructBuffer : register(u3);

[numthreads(1, 1, 1)]
void CSMain(uint id : SV_DispatchThreadID)
{
    // Copy typed buffer and multyply by 2
    outTypedBuffer[id] = inTypedBuffer[id]*multipliers[0];

    // Copy structured buffer and multiply fields by 3 and 4
    Entry entry = inStructBuffer[id];
    entry.a *= multipliers[1];
    entry.b *= multipliers[2];
    outStructBuffer[id] = entry;
}
