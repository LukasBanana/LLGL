/*
 * D3D12StagingDescriptorHeap.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12StagingDescriptorHeap.h"
#include "../D3D12Device.h"
#include "../../DXCommon/DXCore.h"
#include "../../../Core/CoreUtils.h"


namespace LLGL
{


D3D12StagingDescriptorHeap::D3D12StagingDescriptorHeap(D3D12StagingDescriptorHeap&& rhs) noexcept :
    D3D12DescriptorHeap { std::forward<D3D12DescriptorHeap&&>(rhs) },
    offset_             { rhs.offset_                              }
{
}

D3D12StagingDescriptorHeap& D3D12StagingDescriptorHeap::operator = (D3D12StagingDescriptorHeap&& rhs) noexcept
{
    if (this != &rhs)
    {
        D3D12DescriptorHeap::operator = (std::forward<D3D12DescriptorHeap&&>(rhs));
        offset_ = rhs.offset_;
    }
    return *this;
}

D3D12StagingDescriptorHeap::D3D12StagingDescriptorHeap(
    ID3D12Device*               device,
    D3D12_DESCRIPTOR_HEAP_TYPE  type,
    UINT                        size)
:
    D3D12DescriptorHeap { device, type, size, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE }
{
}

void D3D12StagingDescriptorHeap::Create(
    ID3D12Device*               device,
    D3D12_DESCRIPTOR_HEAP_TYPE  type,
    UINT                        size)
{
    D3D12DescriptorHeap::Create(device, type, size, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    ResetOffset();
}

void D3D12StagingDescriptorHeap::ResetOffset()
{
    offset_ = 0;
}

bool D3D12StagingDescriptorHeap::Capacity(UINT count) const
{
    return (offset_ + count <= GetSize());
}

void D3D12StagingDescriptorHeap::CopyDescriptors(
    ID3D12Device*               device,
    D3D12_CPU_DESCRIPTOR_HANDLE srcDescHandle,
    UINT                        firstDescriptor,
    UINT                        numDescriptors)
{
    /* Get source descriptor CPU handle address */
    D3D12_CPU_DESCRIPTOR_HANDLE dstDescHandle = GetNative()->GetCPUDescriptorHandleForHeapStart();
    dstDescHandle.ptr += (offset_ + firstDescriptor) * GetStride();

    /* Copy descriptors from source to destination descriptor heap */
    device->CopyDescriptorsSimple(numDescriptors, dstDescHandle, srcDescHandle, GetType());
}

D3D12_GPU_DESCRIPTOR_HANDLE D3D12StagingDescriptorHeap::GetGpuHandleWithOffset() const
{
    return D3D12DescriptorHeap::GetGpuHandleWithOffset(offset_);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12StagingDescriptorHeap::GetCpuHandleWithOffset(UINT descriptor) const
{
    return D3D12DescriptorHeap::GetCpuHandleWithOffset(offset_ + descriptor);
}

void D3D12StagingDescriptorHeap::IncrementOffset(UINT stride)
{
    offset_ += stride;
}


} // /namespace LLGL



// ================================================================================
