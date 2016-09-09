/*
 * D3D12IndexBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12IndexBuffer.h"


namespace LLGL
{


void D3D12IndexBuffer::UpdateSubResource(
    ID3D12Device* device, ID3D12GraphicsCommandList* gfxCommandList, ComPtr<ID3D12Resource>& bufferUpload,
    const void* data, UINT bufferSize, UINT64 offset)
{
    hwBuffer.UpdateSubResource(
        device, gfxCommandList, bufferUpload,
        data, bufferSize, offset,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
    );
}


} // /namespace LLGL



// ================================================================================
