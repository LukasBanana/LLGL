/*
 * D3D12ConstantBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12ConstantBuffer.h"
#include "../../../Core/Helper.h"
#include "../../DXCommon/DXCore.h"


namespace LLGL
{


D3D12ConstantBuffer::D3D12ConstantBuffer(ID3D12Device* device, const BufferDescriptor& desc) :
    D3D12Buffer( BufferType::Constant )
{
    /* Create descriptor heap for constant buffer */
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    {
        cbvHeapDesc.Type            = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        cbvHeapDesc.NumDescriptors  = 1;
        cbvHeapDesc.Flags           = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        cbvHeapDesc.NodeMask        = 0;
    }
    auto hr = device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&descHeap_));
    DXThrowIfFailed(hr, "failed to create D3D12 descriptor heap for constant-buffer-view (CBV)");

    /* Create resource and put view */
    CreateResourceAndPutView(device, desc.size);
}

void D3D12ConstantBuffer::UpdateSubresource(const void* data, UINT bufferSize, UINT64 offset)
{
    UpdateDynamicSubresource(data, bufferSize, offset);
}


/*
 * ======= Private: =======
 */

void D3D12ConstantBuffer::CreateResourceAndPutView(ID3D12Device* device, UINT bufferSize)
{
    /* Constant buffers are required to be 256-byte aligned */
    static const UINT alignment = 255;
    bufferSize = (bufferSize + alignment) & ~alignment;

    /* Create hardware resource */
    CreateResource(device, bufferSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);

    /* Create constant buffer view (CBV) */
    D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc;
    {
        viewDesc.BufferLocation = Get()->GetGPUVirtualAddress();
        viewDesc.SizeInBytes    = bufferSize;
    }
    device->CreateConstantBufferView(&viewDesc, descHeap_->GetCPUDescriptorHandleForHeapStart());
}


} // /namespace LLGL



// ================================================================================
