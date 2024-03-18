/*
 * D3D12DescriptorHeap.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12DescriptorHeap.h"
#include "../../DXCommon/DXCore.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/MacroUtils.h"
#include "../../../Core/Assertion.h"


namespace LLGL
{


static const char* ToString(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
    switch (heapType)
    {
        LLGL_CASE_TO_STR( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
        LLGL_CASE_TO_STR( D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER );
        LLGL_CASE_TO_STR( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );
        LLGL_CASE_TO_STR( D3D12_DESCRIPTOR_HEAP_TYPE_DSV );
        default: break;
    }
    return nullptr;
}

ComPtr<ID3D12DescriptorHeap> D3D12DescriptorHeap::CreateNativeOrThrow(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc)
{
    ComPtr<ID3D12DescriptorHeap> descHeap;

    HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(descHeap.ReleaseAndGetAddressOf()));
    if (FAILED(hr))
    {
        std::string contextInfo;
        if (const char* s = ToString(desc.Type))
        {
            contextInfo += "for heap type ";
            contextInfo += s;
        }
        DXThrowIfCreateFailed(hr, "ID3D12DescriptorHeap", contextInfo.c_str());
    }

    return descHeap;
}

static ComPtr<ID3D12DescriptorHeap> CreateDXDescriptorHeapOrThrow(
    ID3D12Device*               device,
    D3D12_DESCRIPTOR_HEAP_TYPE  type,
    UINT                        size,
    D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    {
        heapDesc.Type           = type;
        heapDesc.NumDescriptors = size;
        heapDesc.Flags          = flags;
        heapDesc.NodeMask       = 0;
    }
    return D3D12DescriptorHeap::CreateNativeOrThrow(device, heapDesc);
}

D3D12DescriptorHeap::D3D12DescriptorHeap(D3D12DescriptorHeap&& rhs) noexcept :
    native_ { std::move(rhs.native_) },
    type_   { rhs.type_              },
    size_   { rhs.size_              },
    stride_ { rhs.stride_            }
{
}

D3D12DescriptorHeap& D3D12DescriptorHeap::operator = (D3D12DescriptorHeap&& rhs) noexcept
{
    if (this != &rhs)
    {
        native_ = std::move(rhs.native_);
        type_   = rhs.type_;
        size_   = rhs.size_;
        stride_ = rhs.stride_;
    }
    return *this;
}

D3D12DescriptorHeap::D3D12DescriptorHeap(
    ID3D12Device*               device,
    D3D12_DESCRIPTOR_HEAP_TYPE  type,
    UINT                        size,
    D3D12_DESCRIPTOR_HEAP_FLAGS flags)
:
    native_ { CreateDXDescriptorHeapOrThrow(device, type, size, flags) },
    type_   { type                                                     },
    size_   { size                                                     },
    stride_ { device->GetDescriptorHandleIncrementSize(type)           }
{
}

void D3D12DescriptorHeap::Create(
    ID3D12Device*               device,
    D3D12_DESCRIPTOR_HEAP_TYPE  type,
    UINT                        size,
    D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
    /* Create GPU upload buffer */
    native_ = CreateDXDescriptorHeapOrThrow(device, type, size, flags);

    /* Store new size and reset write offset */
    type_   = type;
    size_   = size;
    stride_ = device->GetDescriptorHandleIncrementSize(type);
}

void D3D12DescriptorHeap::Reset(UINT size)
{
    if (size != size_)
    {
        /* Get device from previously created descriptor */
        LLGL_ASSERT(native_ != nullptr);
        ComPtr<ID3D12Device> device;
        native_->GetDevice(IID_PPV_ARGS(device.ReleaseAndGetAddressOf()));

        /* Create descriptor with new size */
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = native_->GetDesc();
        {
            heapDesc.NumDescriptors = size;
        }
        native_ = D3D12DescriptorHeap::CreateNativeOrThrow(device.Get(), heapDesc);

        /* Store new size */
        size_ = size;
    }
}

void D3D12DescriptorHeap::Reset()
{
    native_.Reset();
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::GetCpuHandleStart() const
{
    /* Return destination descriptor CPU handle address */
    return GetNative()->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::GetCpuHandleWithOffset(UINT offset) const
{
    /* Return destination descriptor CPU handle address */
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle = GetNative()->GetCPUDescriptorHandleForHeapStart();
    cpuDescHandle.ptr += offset * GetStride();
    return cpuDescHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::GetGpuHandleStart() const
{
    /* Return destination descriptor GPU handle address */
    return GetNative()->GetGPUDescriptorHandleForHeapStart();
}

D3D12_GPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::GetGpuHandleWithOffset(UINT offset) const
{
    /* Return destination descriptor GPU handle address */
    D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle = GetNative()->GetGPUDescriptorHandleForHeapStart();
    gpuDescHandle.ptr += offset * GetStride();
    return gpuDescHandle;
}


} // /namespace LLGL



// ================================================================================
