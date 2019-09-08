/*
 * D3D12Buffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12Buffer.h"
#include "../D3DX12/d3dx12.h"
#include "../D3D12Types.h"
#include "../D3D12ObjectUtils.h"
#include "../Command/D3D12CommandContext.h"
#include "../../DXCommon/DXCore.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/Helper.h"
#include <stdexcept>


namespace LLGL
{


// BufferFilledSize actually only needs 4 bytes, but we keep it 16 byte aligned
// see https://docs.microsoft.com/en-us/windows/win32/direct3d12/stream-output-counters#bufferfilledsize
static const UINT64 g_soBufferFillSizeLen   = 16u;
static const UINT64 g_cBufferAlignment      = 256u;

D3D12Buffer::D3D12Buffer(ID3D12Device* device, const BufferDescriptor& desc) :
    Buffer { desc.bindFlags }
{
    /* Constant buffers must be aligned to 256 bytes */
    if ((desc.bindFlags & BindFlags::ConstantBuffer) != 0)
        alignment_ = g_cBufferAlignment;

    /* Create native buffer resource */
    CreateGpuBuffer(device, desc);

    /* Create CPU access buffer */
    if (desc.cpuAccessFlags != 0)
        CreateCpuAccessBuffer(device, desc.size, desc.cpuAccessFlags);

    /* Create sub-resource views */
    if ((desc.bindFlags & BindFlags::VertexBuffer) != 0)
        CreateVertexBufferView(desc);
    if ((desc.bindFlags & BindFlags::IndexBuffer) != 0)
        CreateIndexBufferView(desc);
    if ((desc.bindFlags & BindFlags::StreamOutputBuffer) != 0)
        CreateStreamOutputBufferView(desc);
}

void D3D12Buffer::SetName(const char* name)
{
    D3D12SetObjectName(resource_.Get(), name);
}

BufferDescriptor D3D12Buffer::GetDesc() const
{
    /* Get native resource descriptor and convert */
    D3D12_RESOURCE_DESC nativeDesc;
    nativeDesc = GetResource().native->GetDesc();

    BufferDescriptor bufferDesc;
    bufferDesc.size             = nativeDesc.Width;
    bufferDesc.bindFlags        = GetBindFlags();
    #if 0//TODO
    bufferDesc.cpuAccessFlags   = 0;
    bufferDesc.miscFlags        = 0;
    #endif

    return bufferDesc;
}

void D3D12Buffer::CreateConstantBufferView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle)
{
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
    {
        cbvDesc.BufferLocation  = GetNative()->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes     = static_cast<UINT>(GetBufferSize());
    }
    device->CreateConstantBufferView(&cbvDesc, cpuDescHandle);
}

void D3D12Buffer::CreateShaderResourceView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle)
{
    CreateShaderResourceView(device, cpuDescHandle, 0, static_cast<UINT>(bufferSize_ / structStride_), structStride_);
}

void D3D12Buffer::CreateShaderResourceView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, UINT firstElement, UINT numElements, UINT elementStride)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    {
        srvDesc.Format                      = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension               = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Shader4ComponentMapping     = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Buffer.FirstElement         = firstElement;
        srvDesc.Buffer.NumElements          = numElements;
        srvDesc.Buffer.StructureByteStride  = elementStride;
        srvDesc.Buffer.Flags                = D3D12_BUFFER_SRV_FLAG_NONE;
    }
    device->CreateShaderResourceView(GetNative(), &srvDesc, cpuDescHandle);
}

void D3D12Buffer::CreateUnorderedAccessView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle)
{
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    {
        uavDesc.Format                      = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension               = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement         = 0;
        uavDesc.Buffer.NumElements          = static_cast<UINT>(bufferSize_ / structStride_);
        uavDesc.Buffer.StructureByteStride  = structStride_;
        uavDesc.Buffer.CounterOffsetInBytes = 0;
        uavDesc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_NONE;
    }
    device->CreateUnorderedAccessView(GetNative(), nullptr, &uavDesc, cpuDescHandle);
}

static bool HasReadAccess(const CPUAccess access)
{
    return (access == CPUAccess::ReadOnly || access == CPUAccess::ReadWrite);
}

static bool HasWriteAccess(const CPUAccess access)
{
    return (access >= CPUAccess::WriteOnly && access <= CPUAccess::ReadWrite);
}

#define _TEST_USE_COPY_BUFFER_REGION_

HRESULT D3D12Buffer::Map(
    D3D12CommandContext&    commandContext,
    D3D12Fence&             fence,
    const D3D12_RANGE&      range,
    void**                  mappedData,
    const CPUAccess         access)
{
    if (cpuAccessBuffer_.Get() != nullptr)
    {
        /* Store mapped state */
        mappedRange_        = range;
        mappedCPUaccess_    = access;

        if (HasReadAccess(access))
        {
            /* Copy content from GPU host memory to CPU memory */
            commandContext.TransitionResource(resource_, D3D12_RESOURCE_STATE_COPY_SOURCE, true);
            {
                #ifdef _TEST_USE_COPY_BUFFER_REGION_
                commandContext.GetCommandList()->CopyBufferRegion(
                    cpuAccessBuffer_.Get(),
                    range.Begin,
                    GetNative(),
                    range.Begin,
                    range.End - range.Begin
                );
                #else
                commandContext.GetCommandList()->CopyResource(cpuAccessBuffer_.Get(), GetNative());
                #endif
            }
            commandContext.TransitionResource(resource_, resource_.usageState, true);
            commandContext.Finish(&fence);

            /* Map with read range */
            return cpuAccessBuffer_.Get()->Map(0, &range, mappedData);
        }
        else
        {
            /* Map without read range */
            const D3D12_RANGE nullRange{ 0, 0, };
            return cpuAccessBuffer_.Get()->Map(0, &nullRange, mappedData);
        }
    }
    return E_FAIL;
}

void D3D12Buffer::Unmap(
    D3D12CommandContext&    commandContext,
    D3D12Fence&             fence)
{
    if (cpuAccessBuffer_.Get() != nullptr)
    {
        if (HasWriteAccess(mappedCPUaccess_))
        {
            /* Copy content from CPU memory to GPU host memory */
            commandContext.TransitionResource(resource_, D3D12_RESOURCE_STATE_COPY_DEST, true);
            {
                #ifdef _TEST_USE_COPY_BUFFER_REGION_
                commandContext.GetCommandList()->CopyBufferRegion(
                    GetNative(),
                    mappedRange_.Begin,
                    cpuAccessBuffer_.Get(),
                    mappedRange_.Begin,
                    mappedRange_.End - mappedRange_.Begin
                );
                #else
                commandContext.GetCommandList()->CopyResource(GetNative(), cpuAccessBuffer_.Get());
                #endif
            }
            commandContext.TransitionResource(resource_, resource_.usageState, true);
            commandContext.Finish(&fence);

            /* Unmap with written range */
            cpuAccessBuffer_.Get()->Unmap(0, &mappedRange_);
        }
        else
        {
            /* Unmap without written range */
            const D3D12_RANGE nullRange{ 0, 0 };
            cpuAccessBuffer_.Get()->Unmap(0, &nullRange);
        }
    }
}


/*
 * ======= Protected: =======
 */

static D3D12_RESOURCE_FLAGS GetD3DResourceFlags(const BufferDescriptor& desc)
{
    UINT flags = 0;

    if ((desc.bindFlags & BindFlags::Storage) != 0)
        flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    return static_cast<D3D12_RESOURCE_FLAGS>(flags);
}

static D3D12_RESOURCE_STATES GetD3DUsageState(long bindFlags)
{
    D3D12_RESOURCE_STATES flagsD3D = D3D12_RESOURCE_STATE_COMMON;

    if ((bindFlags & (BindFlags::VertexBuffer | BindFlags::ConstantBuffer)) != 0)
        flagsD3D |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    if ((bindFlags & BindFlags::IndexBuffer) != 0)
        flagsD3D |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
    if ((bindFlags & BindFlags::Storage) != 0)
        flagsD3D |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    if ((bindFlags & BindFlags::StreamOutputBuffer) != 0)
        flagsD3D |= D3D12_RESOURCE_STATE_STREAM_OUT;
    if ((bindFlags & BindFlags::IndirectBuffer) != 0)
        flagsD3D |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;

    return flagsD3D;
}

// see https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/nf-d3d12-id3d12device-createcommittedresource
void D3D12Buffer::CreateGpuBuffer(ID3D12Device* device, const BufferDescriptor& desc)
{
    /* Store buffer attributes */
    bufferSize_     = GetAlignedSize<UINT64>(desc.size, alignment_);
    structStride_   = std::max(1u, desc.storageBuffer.stride);

    /* Determine actual resource size */
    internalSize_ = bufferSize_;
    if ((desc.bindFlags & BindFlags::StreamOutputBuffer) != 0)
        internalSize_ += g_soBufferFillSizeLen;

    /* Determine initial resource state */
    resource_.usageState        = GetD3DUsageState(desc.bindFlags);
    resource_.transitionState   = D3D12_RESOURCE_STATE_COPY_DEST;

    /* Create generic buffer resource */
    auto hr = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(GetInternalBufferSize(), GetD3DResourceFlags(desc)),
        resource_.transitionState,
        nullptr,
        IID_PPV_ARGS(resource_.native.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", "for D3D12 hardware buffer");
}

void D3D12Buffer::CreateCpuAccessBuffer(ID3D12Device* device, UINT64 size, long cpuAccessFlags)
{
    /* Determine heap type and resource state */
    D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_UPLOAD;

    if ((cpuAccessFlags & CPUAccessFlags::Write) != 0)
    {
        /* Use upload heap for write and read/write CPU access buffers */
        heapType = D3D12_HEAP_TYPE_UPLOAD;
        cpuAccessBuffer_.SetInitialState(D3D12_RESOURCE_STATE_GENERIC_READ);
    }
    else
    {
        /* Use readback heap for read-only CPU access buffers */
        heapType = D3D12_HEAP_TYPE_READBACK;
        cpuAccessBuffer_.SetInitialState(D3D12_RESOURCE_STATE_COPY_DEST);
    }

    /* Create CPU access buffer */
    auto hr = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(heapType),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(GetInternalBufferSize()),
        cpuAccessBuffer_.usageState,
        nullptr,
        IID_PPV_ARGS(cpuAccessBuffer_.native.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", "for D3D12 CPU access buffer");
}

void D3D12Buffer::CreateVertexBufferView(const BufferDescriptor& desc)
{
    vertexBufferView_.BufferLocation    = GetNative()->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes       = static_cast<UINT>(GetBufferSize());
    vertexBufferView_.StrideInBytes     = (desc.vertexAttribs.empty() ? 0 : desc.vertexAttribs.front().stride);
}

void D3D12Buffer::CreateIndexBufferView(const BufferDescriptor& desc)
{
    indexBufferView_.BufferLocation = GetNative()->GetGPUVirtualAddress();
    indexBufferView_.SizeInBytes    = static_cast<UINT>(GetBufferSize());
    indexBufferView_.Format         = D3D12Types::Map(desc.indexFormat);
}

void D3D12Buffer::CreateStreamOutputBufferView(const BufferDescriptor& desc)
{
    /* Use first 64 bits to buffer fill size, i.e. <BufferFilledSizeLocation>, set buffer location after the first 64 bits */
    soBufferView_.BufferLocation            = GetNative()->GetGPUVirtualAddress();
    soBufferView_.SizeInBytes               = GetBufferSize();
    soBufferView_.BufferFilledSizeLocation  = GetNative()->GetGPUVirtualAddress() + GetBufferSize();
}


} // /namespace LLGL



// ================================================================================
