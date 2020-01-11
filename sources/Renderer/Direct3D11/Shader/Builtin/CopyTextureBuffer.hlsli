/*
 * CopyTextureBuffer.hlsli
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

/* Copy descriptor constant buffer */
cbuffer CopyDescriptor : register(b0)
{
    uint3   texOffset;
    uint    bufOffset;      // Source buffer offset: multiple of 4
    uint3   texExtent;
    uint    bufIndexStride; // Source index stride: 4, 8, 16, 32
    uint    formatSize;     // Bytes per pixel: 1, 2, 4, 8, 12, 16
    uint    components;     // Destination color components: 1, 2, 3, 4
    uint    componentBits;  // Bits per component: 8, 16, 32
    uint    rowStride;
    uint    layerStride;
};

