/*
 * ByteBuffer.hlsl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#define ASCII_UCASE_A 0x41
#define ASCII_UCASE_Z 0x5A
#define ASCII_LCASE_A 0x61
#define ASCII_LCASE_Z 0x7A

ByteAddressBuffer   inByteBufferA  : register(t0); // Arbitrary byte buffer
RWByteAddressBuffer outByteBufferB : register(u1); // Arbitrary byte buffer
RWByteAddressBuffer outByteBufferC : register(u2); // String buffer

[numthreads(64, 1, 1)]
void CSMain(uint id : SV_DispatchThreadID)
{
    // Out of bounds check
    uint numBytes = 0;
    inByteBufferA.GetDimensions(numBytes);

    uint address = id*4;
    if (address >= numBytes)
        return;

    // Read input buffer A
    uint value0 = inByteBufferA.Load(address);

    // Write output buffer B
    outByteBufferB.Store(address, value0 ^ 0xDEADBEEF);

    // Modify output buffer C by treating it as a string and make everything upper case
    uint word = outByteBufferC.Load(address);
    for (int i = 0; i < 4; ++i)
    {
        uint lcaseChar = (word >> i*8) & 0xFF;
        if (lcaseChar >= ASCII_LCASE_A && lcaseChar <= ASCII_LCASE_Z)
        {
            uint ucaseChar = lcaseChar - (ASCII_LCASE_A - ASCII_UCASE_A);
            word = (word ^ (lcaseChar << i*8)) | (ucaseChar << i*8);
        }
    }
    outByteBufferC.Store(address, word);
}
