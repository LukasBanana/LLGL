/*
 * D3D12IndexBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12IndexBuffer.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


D3D12IndexBuffer::D3D12IndexBuffer()
{
    InitMemory(view_);
}

void D3D12IndexBuffer::UpdateSubresource(
    ID3D12Device* device, ID3D12GraphicsCommandList* gfxCommandList, ComPtr<ID3D12Resource>& bufferUpload,
    const void* data, UINT bufferSize, UINT64 offset)
{
    hwBuffer.UpdateStaticSubresource(
        device, gfxCommandList, bufferUpload,
        data, bufferSize, offset,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
    );
}

void D3D12IndexBuffer::PutView(DXGI_FORMAT format)
{
    view_.BufferLocation    = hwBuffer.Get()->GetGPUVirtualAddress();
    view_.SizeInBytes       = hwBuffer.GetBufferSize();
    view_.Format            = format;
}


} // /namespace LLGL



// ================================================================================
