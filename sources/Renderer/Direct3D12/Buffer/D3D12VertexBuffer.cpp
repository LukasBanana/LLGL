/*
 * D3D12VertexBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12VertexBuffer.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


D3D12VertexBuffer::D3D12VertexBuffer(ID3D12Device* device, const BufferDescriptor& desc) :
    D3D12Buffer { BufferType::Vertex }
{
    /* Create resource and initialize buffer view */
    CreateResource(device, desc.size);

    view_.BufferLocation    = Get()->GetGPUVirtualAddress();
    view_.SizeInBytes       = static_cast<UINT>(GetBufferSize());
    view_.StrideInBytes     = desc.vertexBuffer.format.stride;
}

void D3D12VertexBuffer::UpdateSubresource(
    ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ComPtr<ID3D12Resource>& uploadBuffer,
    const void* data, UINT bufferSize, UINT64 offset)
{
    UpdateStaticSubresource(
        device, commandList, uploadBuffer, data, bufferSize, offset, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
    );
}


} // /namespace LLGL



// ================================================================================
