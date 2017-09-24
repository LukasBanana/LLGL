/*
 * D3D12IndexBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12IndexBuffer.h"
#include "../D3D12Types.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


D3D12IndexBuffer::D3D12IndexBuffer(ID3D12Device* device, const BufferDescriptor& desc) :
    D3D12Buffer { BufferType::Index }
{
    /* Create resource and initialize buffer view */
    CreateResource(device, desc.size);

    view_.BufferLocation    = Get()->GetGPUVirtualAddress();
    view_.SizeInBytes       = static_cast<UINT>(GetBufferSize());
    view_.Format            = D3D12Types::Map(desc.indexBuffer.format.GetDataType());
}

void D3D12IndexBuffer::UpdateSubresource(
    ID3D12Device* device, ID3D12GraphicsCommandList* gfxCommandList, ComPtr<ID3D12Resource>& bufferUpload,
    const void* data, UINT bufferSize, UINT64 offset)
{
    UpdateStaticSubresource(
        device, gfxCommandList, bufferUpload,
        data, bufferSize, offset,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
    );
}


} // /namespace LLGL



// ================================================================================
