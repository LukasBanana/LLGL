/*
 * CopyTextureFromBuffer.hlsl
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CopyTextureBuffer.hlsli"


/* Source buffer and destination texture (must have typeless format, e.g. DXGI_FORMAT_R8G8B8A8_TYPELESS) */
ByteAddressBuffer srcBuffer : register(t0);

#if TEXTURE_DIM == 1
RWTexture1DArray<uint4> dstTexture : register(u0);
#elif TEXTURE_DIM == 2
RWTexture2DArray<uint4> dstTexture : register(u0);
#elif TEXTURE_DIM == 3
RWTexture3D<uint4> dstTexture : register(u0);
#endif


void ReadSourceBufferR(uint addr, uint idx, inout uint4 result)
{
    switch (componentBits)
    {
        case 8:
        {
            /* Next 4 consecutive threads read the same DWORD and split it up into BYTE */
            uint value = srcBuffer.Load(addr + idx / 4 * bufIndexStride);
            uint shift = (idx % 4) * 8;
            result.r = ((value >> shift) & 0xFF);
        }
        break;
        
        case 16:
        {
            /* Next 2 consecutive threads read the same DWORD and split it up into WORD */
            uint value = srcBuffer.Load(addr + idx / 2 * bufIndexStride);
            uint shift = (idx % 2) * 16;
            result.r = ((value >> shift) & 0xFFFF);
        }
        break;
        
        case 32:
        {
            result.r = srcBuffer.Load(addr + idx * bufIndexStride);
        }
        break;
    }
}

void ReadSourceBufferRG(uint addr, uint idx, inout uint4 result)
{
    switch (componentBits)
    {
        case 8:
        {
            /* Next 2 consecutive threads read the same DWORD and split it up into WORD */
            uint value = srcBuffer.Load(addr + idx / 2 * bufIndexStride);
            uint shift = (idx % 2) * 16;
            value >>= shift;
            result.r = ((value     ) & 0xFF);
            result.g = ((value >> 8) & 0xFF);
        }
        break;
        
        case 16:
        {
            uint value = srcBuffer.Load(addr + idx * bufIndexStride);
            result.r = ((value      ) & 0xFFFF);
            result.g = ((value >> 16)         );
        }
        break;
        
        case 32:
        {
            result.rg = srcBuffer.Load2(addr + idx * bufIndexStride);
        }
        break;
    }
}

void ReadSourceBufferRGB(uint addr, uint idx, inout uint4 result)
{
    switch (componentBits)
    {
        case 32:
        {
            result.rgb = srcBuffer.Load3(addr + idx * bufIndexStride);
        }
        break;
    }
}

void ReadSourceBufferRGBA(uint addr, uint idx, inout uint4 result)
{
    switch (componentBits)
    {
        case 8:
        {
            uint value = srcBuffer.Load(addr + idx * bufIndexStride);
            result.r = ((value      ) & 0xFF);
            result.g = ((value >>  8) & 0xFF);
            result.b = ((value >> 16) & 0xFF);
            result.a = ((value >> 24)       );
        }
        break;
        
        case 16:
        {
            uint2 value = srcBuffer.Load2(addr + idx * bufIndexStride);
            result.r = ((value.r      ) & 0xFFFF);
            result.g = ((value.r >> 16)         );
            result.b = ((value.g      ) & 0xFFFF);
            result.a = ((value.g >> 16)         );
        }
        break;
        
        case 32:
        {
            result = srcBuffer.Load4(addr + idx * bufIndexStride);
        }
        break;
    }
}

/* Read source buffer with dynamic format */
uint4 ReadSourceBuffer(uint idx, uint3 coord)
{
    /* Start reading address at buffer offset */
    uint addr = bufOffset;
    
    /* Add row padding */
    uint rowSize    = texExtent.x * formatSize;
    uint rowPadding = (rowStride > rowSize ? rowStride - rowSize : 0);
    addr += rowPadding * coord.y;
    
    /* Add layer padding */
    uint layerSize      = (rowSize + rowPadding) * texExtent.y;
    uint layerPadding   = (layerStride > layerSize ? layerStride - layerSize : 0);
    addr += layerPadding * coord.z;
    
    /* Read source value */
    uint4 result = (uint4)0;
    
    switch (components)
    {
        case 1:
            ReadSourceBufferR(addr, idx, result);
            break;
        
        case 2:
            ReadSourceBufferRG(addr, idx, result);
            break;
        
        case 3:
            ReadSourceBufferRGB(addr, idx, result);
            break;
        
        case 4:
            ReadSourceBufferRGBA(addr, idx, result);
            break;
    }
    
    return result;
}

/* Write destination texture with dynamic format */
void WriteDestinationTexutre(uint3 coord, uint4 value)
{
    /* Calculate texture coordinate from linear index */
    uint3 pos = texOffset + coord;
    
    /* Write components to destination texture */
    #if TEXTURE_DIM == 1
    dstTexture[pos.xy] = value;
    #else
    dstTexture[pos] = value;
    #endif
}

/* Primary compute kernel to copy buffer value into texel */
[numthreads(1, 1, 1)]
void CopyTextureFromBuffer(uint3 threadID : SV_DispatchThreadID)
{
    /* Flatten global thread ID */
    uint groupIndex = threadID.x + (threadID.y + threadID.z * texExtent.y) * texExtent.x;
    
    /* Read value from source buffer */
    uint4 value = ReadSourceBuffer(groupIndex, threadID);
    
    /* Write value to destination texture */
    WriteDestinationTexutre(threadID, value);
}

