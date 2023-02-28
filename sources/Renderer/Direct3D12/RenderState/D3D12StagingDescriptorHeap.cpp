/*
 * D3D12StagingDescriptorHeap.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12StagingDescriptorHeap.h"
#include "../D3D12Device.h"
#include "../../DXCommon/DXCore.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


D3D12StagingDescriptorHeap::D3D12StagingDescriptorHeap(
    D3D12Device&                device,
    D3D12_DESCRIPTOR_HEAP_TYPE  type,
    UINT                        size)
{
    Create(device, type, size);
}

D3D12StagingDescriptorHeap::D3D12StagingDescriptorHeap(D3D12StagingDescriptorHeap&& rhs) :
    native_ { std::move(rhs.native_) },
    size_   { rhs.size_              },
    offset_ { rhs.offset_            }
{
}

D3D12StagingDescriptorHeap& D3D12StagingDescriptorHeap::operator = (D3D12StagingDescriptorHeap&& rhs)
{
    if (this != &rhs)
    {
        native_ = std::move(rhs.native_);
        size_   = rhs.size_;
        offset_ = rhs.offset_;
    }
    return *this;
}

void D3D12StagingDescriptorHeap::Create(
    D3D12Device&                device,
    D3D12_DESCRIPTOR_HEAP_TYPE  type,
    UINT                        size)
{
    /* Create GPU upload buffer */
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    {
        heapDesc.Type           = type;
        heapDesc.NumDescriptors = size;
        heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        heapDesc.NodeMask       = 0;
    }
    native_ = device.CreateDXDescriptorHeap(heapDesc);

    /* Store new size and reset write offset */
    type_   = type;
    size_   = size;
    offset_ = 0;
    stride_ = device.GetNative()->GetDescriptorHandleIncrementSize(type);
}

void D3D12StagingDescriptorHeap::Reset()
{
    offset_ = 0;
}

bool D3D12StagingDescriptorHeap::Capacity(UINT count) const
{
    return (offset_ + count <= size_);
}

void D3D12StagingDescriptorHeap::CopyDescriptors(
    ID3D12Device*               device,
    D3D12_CPU_DESCRIPTOR_HANDLE srcDescHandle,
    UINT                        firstDescriptor,
    UINT                        numDescriptors)
{
    /* Get source descriptor CPU handle address */
    D3D12_CPU_DESCRIPTOR_HANDLE dstDescHandle = native_->GetCPUDescriptorHandleForHeapStart();
    dstDescHandle.ptr += (offset_ + firstDescriptor) * stride_;

    /* Copy descriptors from source to destination descriptor heap */
    device->CopyDescriptorsSimple(numDescriptors, dstDescHandle, srcDescHandle, type_);
}

D3D12_GPU_DESCRIPTOR_HANDLE D3D12StagingDescriptorHeap::GetGPUDescriptorHandleForHeapStart() const
{
    /* Return destination descriptor GPU handle address */
    D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle = native_->GetGPUDescriptorHandleForHeapStart();
    gpuDescHandle.ptr += offset_ * stride_;
    return gpuDescHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12StagingDescriptorHeap::GetCPUDescriptorHandle(UINT descriptor) const
{
    /* Return destination descriptor CPU handle address */
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle = native_->GetCPUDescriptorHandleForHeapStart();
    cpuDescHandle.ptr += (offset_ + descriptor) * stride_;
    return cpuDescHandle;
}

void D3D12StagingDescriptorHeap::IncrementOffset(UINT stride)
{
    offset_ += stride;
}


} // /namespace LLGL



// ================================================================================
