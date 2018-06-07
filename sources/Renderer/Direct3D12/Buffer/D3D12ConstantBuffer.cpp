/*
 * D3D12ConstantBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12ConstantBuffer.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


D3D12ConstantBuffer::D3D12ConstantBuffer(ID3D12Device* device, const BufferDescriptor& desc) :
    D3D12Buffer { BufferType::Constant }
{
    CreateResourceWithAlignment(device, static_cast<UINT>(desc.size));
}

void D3D12ConstantBuffer::UpdateSubresource(const void* data, UINT bufferSize, UINT64 offset)
{
    UpdateDynamicSubresource(data, bufferSize, offset);
}

void D3D12ConstantBuffer::CreateResourceView(ID3D12Device* device, ID3D12DescriptorHeap* descriptorHeap)
{
    /* Create constant buffer view (CBV) */
    D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc;
    {
        viewDesc.BufferLocation = GetNative()->GetGPUVirtualAddress();
        viewDesc.SizeInBytes    = bufferSize_;
    }
    device->CreateConstantBufferView(&viewDesc, descriptorHeap->GetCPUDescriptorHandleForHeapStart());
}


/*
 * ======= Private: =======
 */

void D3D12ConstantBuffer::CreateResourceWithAlignment(ID3D12Device* device, UINT bufferSize)
{
    /* Constant buffers are required to be 256-byte aligned */
    static const UINT alignment = 256;
    bufferSize_ = GetAlignedSize(bufferSize, alignment);

    /* Create hardware resource */
    CreateResource(device, bufferSize_, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);
}


} // /namespace LLGL



// ================================================================================
