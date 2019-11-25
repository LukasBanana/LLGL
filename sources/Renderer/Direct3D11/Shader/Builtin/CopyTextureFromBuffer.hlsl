/*
 * CopyTextureFromBuffer.hlsl
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

/* Current MIP-map level configuration */
cbuffer CopyDescriptor : register(b0)
{
    uint3   dstOffset;
    uint    srcOffset;      // Source buffer offset: multiple of 4
    uint3   dstExtent;
    uint    srcIndexStride; // Source index stride: 4, 8, 16, 32
    uint    formatSize;     // Bytes per pixel: 1, 2, 4, 8, 12, 16
    uint    components;     // Destination color components: 1, 2, 3, 4
    uint    componentBits;  // Bits per component: 8, 16, 32
    uint    rowStride;
    uint    layerStride;
};


/* Destination texture (must have typeless format, e.g. DXGI_FORMAT_R8G8B8A8_TYPELESS) and source buffer */
ByteAddressBuffer srcBuffer : register(t0);

#if TEXTURE_DIM == 1
RWTexture1DArray<uint4> dstTexture : register(u0);
#elif TEXTURE_DIM == 2
RWTexture2DArray<uint4> dstTexture : register(u0);
#elif TEXTURE_DIM == 3
RWTexture3D<uint4> dstTexture : register(u0);
#endif


void ReadSourceBufferR(inout uint4 result, uint addr, uint idx)
{
    switch (componentBits)
    {
        case 8:
        {
            uint value = srcBuffer.Load(addr);
            uint shift = (idx % 4) * 8;
            result.r = ((value >> shift) & 0xFF);
        }
        break;
        
        case 16:
        {
            uint value = srcBuffer.Load(addr);
            uint shift = (idx % 2) * 8;
            result.r = ((value >> shift) & 0xFFFF);
        }
        break;
        
        case 32:
        {
            result.r = srcBuffer.Load(addr);
        }
        break;
    }
}

void ReadSourceBufferRG(inout uint4 result, uint addr, uint idx)
{
    switch (componentBits)
    {
        case 8:
        {
            uint value = srcBuffer.Load(addr);
            uint shift = ((idx / 2) % 2) * 16;
            value >>= shift;
            result.r = ((value     ) & 0xFF);
            result.g = ((value >> 8) & 0xFF);
        }
        break;
        
        case 16:
        {
            uint value = srcBuffer.Load(addr);
            result.r = ((value      ) & 0xFFFF);
            result.g = ((value >> 16)         );
        }
        break;
        
        case 32:
        {
            result.rg = srcBuffer.Load2(addr);
        }
        break;
    }
}

void ReadSourceBufferRGB(inout uint4 result, uint addr)
{
    switch (componentBits)
    {
        case 32:
        {
            result.rgb = srcBuffer.Load3(addr);
        }
        break;
    }
}

void ReadSourceBufferRGBA(inout uint4 result, uint addr)
{
    switch (componentBits)
    {
        case 8:
        {
            uint value = srcBuffer.Load(addr);
            result.r = ((value      ) & 0xFF);
            result.g = ((value >>  8) & 0xFF);
            result.b = ((value >> 16) & 0xFF);
            result.a = ((value >> 24)       );
        }
        break;
        
        case 16:
        {
            uint2 value = srcBuffer.Load2(addr);
            result.r = ((value.r      ) & 0xFFFF);
            result.g = ((value.r >> 16)         );
            result.b = ((value.g      ) & 0xFFFF);
            result.a = ((value.g >> 16)         );
        }
        break;
        
        case 32:
        {
            result = srcBuffer.Load4(addr);
        }
        break;
    }
}

/* Read source buffer with dynamic format */
uint4 ReadSourceBuffer(uint idx, uint3 coord)
{
    /* Calculate source buffer address from linear index */
    uint addr = srcOffset + idx * srcIndexStride;
    
    /* Add row padding */
    uint rowSize    = dstExtent.x * formatSize;
    uint rowPadding = (rowStride > rowSize ? rowStride - rowSize : 0);
    addr += rowPadding * coord.y;
    
    /* Add layer padding */
    uint layerSize      = (rowSize + rowPadding) * dstExtent.y;
    uint layerPadding   = (layerStride > layerSize ? layerStride - layerSize : 0);
    addr += layerPadding * coord.z;
    
    /* Read source value */
    uint4 result = (uint4)0;
    
    switch (components)
    {
        case 1:
            ReadSourceBufferR(result, addr, idx);
            break;
        
        case 2:
            ReadSourceBufferRG(result, addr, idx);
            break;
        
        case 3:
            ReadSourceBufferRGB(result, addr);
            break;
        
        case 4:
            ReadSourceBufferRGBA(result, addr);
            break;
    }
    
    return result;
}

/* Write destination texture with dynamic format */
void WriteDestinationTexutre(uint3 coord, uint4 value)
{
    /* Calculate texture coordinate from linear index */
    uint3 pos = dstOffset + coord;
    
    /* Write components to destination texture */
    #if TEXTURE_DIM == 1
    dstTexture[pos.xy] = value;
    #else
    dstTexture[pos] = value;
    #endif
}


/* Primary compute kernel to up to 4 MIP-map levels at a time */
[numthreads(1, 1, 1)]
void CopyTextureFromBuffer(uint3 threadID : SV_DispatchThreadID)
{
    /* Flatten global thread ID */
    uint groupIndex = threadID.x + (threadID.y + threadID.z * dstExtent.y) * dstExtent.x;
    
    /* Read value from source buffer */
    uint4 value = ReadSourceBuffer(groupIndex, threadID);
    
    /* Write value to destination */
    WriteDestinationTexutre(threadID, value);
}

