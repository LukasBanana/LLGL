/*
 * CopyBufferFromTexture.hlsl
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CopyTextureBuffer.hlsli"


/* Destination buffer and source texture (must have typeless format, e.g. DXGI_FORMAT_R8G8B8A8_TYPELESS) */
RWByteAddressBuffer dstBuffer : register(u0);

#if TEXTURE_DIM == 1
Texture1DArray<uint4> srcTexture : register(t0);
#elif TEXTURE_DIM == 2
Texture2DArray<uint4> srcTexture : register(t0);
#elif TEXTURE_DIM == 3
Texture3D<uint4> srcTexture : register(t0);
#endif


/* Read source texture with dynamic format */
uint4 ReadSourceTexutre(uint3 coord)
{
    /* Calculate texture coordinate from linear index */
    uint3 pos = texOffset + coord;
    
    /* Read components from source texture */
    #if TEXTURE_DIM == 1
    return srcTexture[pos.xy];
    #else
    return srcTexture[pos];
    #endif
}

void WriteDestinationBufferR(uint addr, uint idx, uint4 value)
{
    switch (componentBits)
    {
        case 8:
        {
            //TODO
            //uint shift = (idx % 4) * 8;
            //uint result = ((value.r >> shift) & 0xFF);
            //dstBuffer.Store(addr, result);
        }
        break;
        
        case 16:
        {
            //TODO
            //uint value = dstBuffer.Store(addr);
            //uint shift = (idx % 2) * 8;
            //result.r = ((value >> shift) & 0xFFFF);
        }
        break;
        
        case 32:
        {
            dstBuffer.Store(addr, value.r);
        }
        break;
    }
}

void WriteDestinationBufferRG(uint addr, uint idx, uint4 value)
{
    switch (componentBits)
    {
        case 8:
        {
            //TODO
            //uint value = dstBuffer.Store(addr);
            //uint shift = ((idx / 2) % 2) * 16;
            //value >>= shift;
            //result.r = ((value     ) & 0xFF);
            //result.g = ((value >> 8) & 0xFF);
        }
        break;
        
        case 16:
        {
            uint result =
            (
                ((value.r & 0xFFFF)      ) |
                ((value.g & 0xFFFF) << 16)
            );
            dstBuffer.Store(addr, result);
        }
        break;
        
        case 32:
        {
            dstBuffer.Store2(addr, value.rg);
        }
        break;
    }
}

void WriteDestinationBufferRGB(uint addr, uint4 value)
{
    switch (componentBits)
    {
        case 32:
        {
            dstBuffer.Store3(addr, value.rgb);
        }
        break;
    }
}

void WriteDestinationBufferRGBA(uint addr, uint4 value)
{
    switch (componentBits)
    {
        case 8:
        {
            uint result =
            (
                ((value.r & 0xFF)      ) |
                ((value.g & 0xFF) <<  8) |
                ((value.b & 0xFF) << 16) |
                ((value.a & 0xFF) << 24)
            );
            dstBuffer.Store(addr, result);
        }
        break;
        
        case 16:
        {
            uint2 result;
            result.r =
            (
                ((value.r & 0xFFFF)      ) |
                ((value.g & 0xFFFF) << 16)
            );
            result.g =
            (
                ((value.b & 0xFFFF)      ) |
                ((value.a & 0xFFFF) << 16)
            );
            dstBuffer.Store2(addr, result);
        }
        break;
        
        case 32:
        {
            dstBuffer.Store4(addr, value);
        }
        break;
    }
}

/* Read source buffer with dynamic format */
void WriteDestinationBuffer(uint idx, uint3 coord, uint4 value)
{
    /* Calculate source buffer address from linear index */
    uint addr = bufOffset + idx * bufIndexStride;
    
    /* Add row padding */
    uint rowSize    = texExtent.x * formatSize;
    uint rowPadding = (rowStride > rowSize ? rowStride - rowSize : 0);
    addr += rowPadding * coord.y;
    
    /* Add layer padding */
    uint layerSize      = (rowSize + rowPadding) * texExtent.y;
    uint layerPadding   = (layerStride > layerSize ? layerStride - layerSize : 0);
    addr += layerPadding * coord.z;
    
    /* Write destination value */
    switch (components)
    {
        case 1:
            WriteDestinationBufferR(addr, idx, value);
            break;
        
        case 2:
            WriteDestinationBufferRG(addr, idx, value);
            break;
        
        case 3:
            WriteDestinationBufferRGB(addr, value);
            break;
        
        case 4:
            WriteDestinationBufferRGBA(addr, value);
            break;
    }
}


/* Primary compute kernel to up to 4 MIP-map levels at a time */
[numthreads(1, 1, 1)]
void CopyBufferFromTexture(uint3 threadID : SV_DispatchThreadID)
{
    /* Flatten global thread ID */
    uint groupIndex = threadID.x + (threadID.y + threadID.z * texExtent.y) * texExtent.x;
    
    /* Read value from source texture */
    uint4 value = ReadSourceTexutre(threadID);
    
    /* Write value to destination buffer */
    WriteDestinationBuffer(groupIndex, threadID, value);
}

