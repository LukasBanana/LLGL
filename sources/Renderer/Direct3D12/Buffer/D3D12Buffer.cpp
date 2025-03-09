/*
 * D3D12Buffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12Buffer.h"
#include "D3D12StagingBufferPool.h"
#include "../D3DX12/d3dx12.h"
#include "../D3D12Types.h"
#include "../D3D12ObjectUtils.h"
#include "../Command/D3D12CommandContext.h"
#include "../Command/D3D12CommandQueue.h"
#include "../../DXCommon/DXCore.h"
#include "../../BufferUtils.h"
#include "../../ResourceUtils.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/CoreUtils.h"
#include <LLGL/Backend/Direct3D12/NativeHandle.h>
#include <stdexcept>


namespace LLGL
{


// BufferFilledSize actually only needs 4 bytes, but we keep it 16 byte aligned
// see https://docs.microsoft.com/en-us/windows/win32/direct3d12/stream-output-counters#bufferfilledsize
static constexpr UINT64 g_soBufferFillSizeLen   = sizeof(UINT64);
static constexpr UINT64 g_cBufferAlignment      = 256u;

// Returns DXGI_FORMAT_UNKNOWN for a structured buffer, or maps the format attribute to DXGI_FORMAT enum.
static DXGI_FORMAT GetDXFormatForBuffer(const BufferDescriptor& desc)
{
    return (IsStructuredBuffer(desc) ? DXGI_FORMAT_UNKNOWN : DXTypes::ToDXGIFormat(desc.format));
}

D3D12Buffer::D3D12Buffer(ID3D12Device* device, const BufferDescriptor& desc) :
    Buffer  { desc.bindFlags             },
    format_ { GetDXFormatForBuffer(desc) }
{
    /* Constant buffers must be aligned to 256 bytes */
    if ((desc.bindFlags & BindFlags::ConstantBuffer) != 0)
        alignment_ = g_cBufferAlignment;

    /* Create native buffer resource */
    CreateGpuBuffer(device, desc);

    /* Create sub-resource views */
    if ((desc.bindFlags & BindFlags::VertexBuffer) != 0)
        CreateVertexBufferView(desc);
    if ((desc.bindFlags & BindFlags::IndexBuffer) != 0)
        CreateIndexBufferView(desc);
    if ((desc.bindFlags & BindFlags::StreamOutputBuffer) != 0)
        CreateStreamOutputBufferView(desc);

    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

bool D3D12Buffer::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (auto* nativeHandleD3D = GetTypedNativeHandle<Direct3D12::ResourceNativeHandle>(nativeHandle, nativeHandleSize))
    {
        nativeHandleD3D->type                   = Direct3D12::ResourceNativeType::SamplerDescriptor;
        nativeHandleD3D->resource.resource      = resource_.Get();
        nativeHandleD3D->resource.resourceState = resource_.currentState;
        nativeHandleD3D->resource.resource->AddRef();
        return true;
    }
    return false;
}

void D3D12Buffer::SetDebugName(const char* name)
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
    CreateConstantBufferViewPrimary(
        device,
        cpuDescHandle,
        0,
        static_cast<UINT>(GetBufferSize())
    );
}

void D3D12Buffer::CreateConstantBufferView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, const BufferViewDescriptor& bufferViewDesc)
{
    CreateConstantBufferViewPrimary(
        device,
        cpuDescHandle,
        bufferViewDesc.offset,
        static_cast<UINT>(std::min(bufferViewDesc.size, GetBufferSize()))
    );
}

//private
void D3D12Buffer::CreateConstantBufferViewPrimary(
    ID3D12Device*               device,
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle,
    UINT64                      offset,
    UINT                        size)
{
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
    {
        cbvDesc.BufferLocation  = GetNative()->GetGPUVirtualAddress() + offset;
        cbvDesc.SizeInBytes     = size;
    }
    device->CreateConstantBufferView(&cbvDesc, cpuDescHandle);
}

void D3D12Buffer::CreateShaderResourceView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle)
{
    CreateShaderResourceViewPrimary(device, cpuDescHandle, 0, static_cast<UINT>(GetBufferSize() / stride_), stride_, format_);
}

void D3D12Buffer::CreateShaderResourceView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, const BufferViewDescriptor& bufferViewDesc)
{
    const UINT      stride          = GetStrideForView(bufferViewDesc.format);
    const UINT64    firstElement    = bufferViewDesc.offset / stride;
    const UINT      numElements     = static_cast<UINT>(std::min(bufferViewDesc.size, GetBufferSize()) / stride);

    CreateShaderResourceViewPrimary(device, cpuDescHandle, firstElement, numElements, stride, DXTypes::ToDXGIFormat(bufferViewDesc.format));
}

//private
void D3D12Buffer::CreateShaderResourceViewPrimary(
    ID3D12Device*               device,
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle,
    UINT64                      firstElement,
    UINT                        numElements,
    UINT                        stride,
    DXGI_FORMAT                 format)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    {
        srvDesc.Format                      = format;
        srvDesc.ViewDimension               = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Shader4ComponentMapping     = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Buffer.FirstElement         = firstElement;
        srvDesc.Buffer.NumElements          = numElements;
        srvDesc.Buffer.StructureByteStride  = (format == DXGI_FORMAT_UNKNOWN ? stride : 0);
        srvDesc.Buffer.Flags                = D3D12_BUFFER_SRV_FLAG_NONE; //TODO: D3D12_BUFFER_SRV_FLAG_RAW
    }
    device->CreateShaderResourceView(GetNative(), &srvDesc, cpuDescHandle);
}

void D3D12Buffer::CreateUnorderedAccessView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle)
{
    CreateUnorderedAccessViewPrimary(device, cpuDescHandle, 0, static_cast<UINT>(GetBufferSize() / stride_), stride_, format_);
}

void D3D12Buffer::CreateUnorderedAccessView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, const BufferViewDescriptor& bufferViewDesc)
{
    const UINT      stride          = GetStrideForView(bufferViewDesc.format);
    const UINT64    firstElement    = bufferViewDesc.offset / stride;
    const UINT      numElements     = static_cast<UINT>(std::min(bufferViewDesc.size, GetBufferSize()) / stride);

    CreateUnorderedAccessViewPrimary(device, cpuDescHandle, firstElement, numElements, stride, DXTypes::ToDXGIFormat(bufferViewDesc.format));
}

//private
//TODO: support counter resource
void D3D12Buffer::CreateUnorderedAccessViewPrimary(
    ID3D12Device*               device,
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle,
    UINT64                      firstElement,
    UINT                        numElements,
    UINT                        stride,
    DXGI_FORMAT                 format)
{
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    {
        uavDesc.Format                      = format;
        uavDesc.ViewDimension               = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement         = firstElement;
        uavDesc.Buffer.NumElements          = numElements;
        uavDesc.Buffer.StructureByteStride  = (format == DXGI_FORMAT_UNKNOWN ? stride : 0);
        uavDesc.Buffer.CounterOffsetInBytes = 0;
        uavDesc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_NONE; //TODO: D3D12_BUFFER_UAV_FLAG_RAW
    }
    device->CreateUnorderedAccessView(GetNative(), nullptr, &uavDesc, cpuDescHandle);
}

void D3D12Buffer::ClearSubresourceUInt(
    D3D12CommandContext&    commandContext,
    DXGI_FORMAT             format,
    UINT                    formatStride,
    UINT64                  offset,
    UINT64                  fillSize,
    const UINT              (&values)[4])
{
    ID3D12GraphicsCommandList*  commandList = commandContext.GetCommandList();
    ID3D12Resource*             resource    = GetNative();

    /* Create intermediate buffer if the primary buffer does not support UAVs */
    const bool useIntermediateBuffer = ((GetBindFlags() & BindFlags::Storage) == 0);

    if (useIntermediateBuffer)
    {
        if (uavIntermediateBuffer_.Get() == nullptr)
            CreateIntermediateUAVBuffer();
        resource = uavIntermediateBuffer_.Get();
    }

    /* Create intermediate descriptor heap if not already done */
    if (!uavIntermediateDescHeap_)
        CreateIntermediateUAVDescriptorHeap(resource, format, formatStride);

    /* Get GPU and CPU descriptor handles for intermediate descriptor heap */
    D3D12DescriptorHeapSetLayout oldLayout;
    D3D12RootParameterIndices oldRootParamIndices;
    commandContext.GetStagingDescriptorHeaps(oldLayout, oldRootParamIndices);

    D3D12DescriptorHeapSetLayout newLayout;
    newLayout.numHeapResourceViews = 1;
    commandContext.SetStagingDescriptorHeaps(newLayout, {});
    //D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle = uavIntermediateDescHeap_->GetGPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle = uavIntermediateDescHeap_->GetCPUDescriptorHandleForHeapStart();
    D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle = commandContext.CopyDescriptorsForStaging(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, cpuDescHandle, 0, 1);

    if (useIntermediateBuffer)
    {
        /* Clear intermediate buffer with UAV */
        const D3D12_RESOURCE_STATES oldIntermediateBufferState = uavIntermediateBuffer_.currentState;
        commandContext.TransitionResource(uavIntermediateBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

        ClearSubresourceWithUAV(
            commandList,
            uavIntermediateBuffer_.Get(),
            GetBufferSize(),
            gpuDescHandle,
            cpuDescHandle,
            offset,
            fillSize,
            formatStride,
            values
        );

        /* Copy intermediate buffer into destination buffer */
        commandContext.TransitionResource(uavIntermediateBuffer_, D3D12_RESOURCE_STATE_COPY_SOURCE);
        commandContext.TransitionResource(GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, true);

        if (fillSize == GetBufferSize())
            commandList->CopyResource(GetNative(), uavIntermediateBuffer_.Get());
        else
            commandList->CopyBufferRegion(GetNative(), offset, uavIntermediateBuffer_.Get(), offset, fillSize);
    }
    else
    {
        /* Clear destination buffer directly with intermediate UAV */
        commandContext.TransitionResource(GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

        ClearSubresourceWithUAV(
            commandList,
            GetNative(),
            GetBufferSize(),
            gpuDescHandle,
            cpuDescHandle,
            offset,
            fillSize,
            formatStride,
            values
        );
    }

    /* Reset previous staging descriptor heaps */
    commandContext.SetStagingDescriptorHeaps(oldLayout, oldRootParamIndices);
}

HRESULT D3D12Buffer::Map(
    D3D12CommandContext&    commandContext,
    D3D12CommandQueue&      commandQueue,
    D3D12StagingBufferPool& stagingBufferPool,
    const D3D12_RANGE&      range,
    void**                  mappedData,
    const CPUAccess         access)
{
    /* Store mapped state */
    mappedRange_        = range;
    mappedCPUaccess_    = access;

    if (access == CPUAccess::ReadWrite)
    {
        /* First map write access buffer */
        SIZE_T rangeSize = range.End - range.Begin;
        mappedBufferTicket_ = stagingBufferPool.MapUploadBuffer(rangeSize, mappedData);
        if (FAILED(mappedBufferTicket_.hr))
            return mappedBufferTicket_.hr;
        if (*mappedData == nullptr)
            return E_FAIL;

        /* Now map feedback buffer and copy its content into the upload buffer */
        void* mappedFeedbackData = nullptr;
        auto readTicket = stagingBufferPool.MapFeedbackBuffer(commandContext, commandQueue, resource_, range, &mappedFeedbackData);
        if (FAILED(readTicket.hr))
            return readTicket.hr;

        ::memcpy(*mappedData, mappedFeedbackData, rangeSize);
        stagingBufferPool.UnmapFeedbackBuffer(readTicket);
    }
    else if (access == CPUAccess::ReadOnly)
    {
        /* Map feedback buffer */
        mappedBufferTicket_ = stagingBufferPool.MapFeedbackBuffer(commandContext, commandQueue, resource_, range, mappedData);
    }
    else
    {
        /* Map upload buffer */
        SIZE_T rangeSize = range.End - range.Begin;
        mappedBufferTicket_ = stagingBufferPool.MapUploadBuffer(rangeSize, mappedData);
    }
    return mappedBufferTicket_.hr;
}

void D3D12Buffer::Unmap(
    D3D12CommandContext&    commandContext,
    D3D12CommandQueue&      commandQueue,
    D3D12StagingBufferPool& stagingBufferPool)
{
    if (HasWriteAccess(mappedCPUaccess_))
        stagingBufferPool.UnmapUploadBuffer(commandContext, commandQueue, resource_, mappedRange_, mappedBufferTicket_);
    else
        stagingBufferPool.UnmapFeedbackBuffer(mappedBufferTicket_);
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

    if (flagsD3D == 0)
    {
        if ((bindFlags & BindFlags::Storage) != 0)
            flagsD3D |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        else if ((bindFlags & BindFlags::StreamOutputBuffer) != 0)
            flagsD3D |= D3D12_RESOURCE_STATE_STREAM_OUT;
        else if ((bindFlags & BindFlags::IndirectBuffer) != 0)
            flagsD3D |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
        else if ((bindFlags & BindFlags::Sampled) != 0)
            flagsD3D |= (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }

    return flagsD3D;
}

// see https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/nf-d3d12-id3d12device-createcommittedresource
void D3D12Buffer::CreateGpuBuffer(ID3D12Device* device, const BufferDescriptor& desc)
{
    /* Store buffer attributes */
    bufferSize_ = GetAlignedSize<UINT64>(desc.size, alignment_);
    stride_     = GetStorageBufferStride(desc);

    /* Determine actual resource size */
    if ((desc.bindFlags & BindFlags::StreamOutputBuffer) != 0)
    {
        internalSize_   = bufferSize_ + g_soBufferFillSizeLen;
        stride_         = (!desc.vertexAttribs.empty() ? desc.vertexAttribs[0].stride : 0);
    }
    else
        internalSize_ = bufferSize_;

    /* Store buffer primary usage stage */
    resource_.usageState = GetD3DUsageState(desc.bindFlags);

    /* Create generic buffer resource */
    const CD3DX12_HEAP_PROPERTIES heapProperties{ D3D12_HEAP_TYPE_DEFAULT };
    const CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(GetInternalBufferSize(), GetD3DResourceFlags(desc));
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COMMON, // Buffers are effectively created in D3D12_RESOURCE_STATE_COMMON state
        nullptr,
        IID_PPV_ARGS(resource_.native.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", "for D3D12 hardware buffer");
}

/*
TODO: Needs refactoring:
    When the format changes, a new UAV entry must be put into the descriptor heap, so the heap must also ne able to grow.
*/
void D3D12Buffer::CreateIntermediateUAVDescriptorHeap(ID3D12Resource* resource, DXGI_FORMAT format, UINT formatStride)
{
    /* Use device the resource was created with */
    ComPtr<ID3D12Device> device;
    resource_.Get()->GetDevice(IID_PPV_ARGS(&device));

    /* Create intermediate descriptor heap only for this resource */
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    {
        heapDesc.Type               = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heapDesc.NumDescriptors     = 1;
        heapDesc.Flags              = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        heapDesc.NodeMask           = 0;
    }
    HRESULT hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(uavIntermediateDescHeap_.ReleaseAndGetAddressOf()));
    DXThrowIfCreateFailed(hr, "ID3D12DescriptorHeap", "for buffer subresource UAV");

    /* Create UAV for subresource in intermediate heap descriptor */
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    {
        uavDesc.ViewDimension               = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Format                      = format;
        uavDesc.Buffer.FirstElement         = 0;
        uavDesc.Buffer.NumElements          = static_cast<UINT>(bufferSize_ / formatStride);
        uavDesc.Buffer.StructureByteStride  = 0;
        uavDesc.Buffer.CounterOffsetInBytes = 0;
        uavDesc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_NONE;
    }
    device->CreateUnorderedAccessView(resource, nullptr, &uavDesc, uavIntermediateDescHeap_->GetCPUDescriptorHandleForHeapStart());
}

void D3D12Buffer::CreateIntermediateUAVBuffer()
{
    /* Use device the resource was created with */
    ComPtr<ID3D12Device> device;
    resource_.Get()->GetDevice(IID_PPV_ARGS(&device));

    /* Create intermediate resource with UAV support */
    const CD3DX12_HEAP_PROPERTIES heapProperties{ D3D12_HEAP_TYPE_DEFAULT };
    const CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(GetInternalBufferSize(), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COMMON, // Buffers are effectively created in D3D12_RESOURCE_STATE_COMMON state
        nullptr,
        IID_PPV_ARGS(uavIntermediateBuffer_.native.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", "for buffer subresource UAV");
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
    indexBufferView_.Format         = DXTypes::ToDXGIFormat(desc.format);
}

void D3D12Buffer::CreateStreamOutputBufferView(const BufferDescriptor& desc)
{
    /* Use first 64 bits to buffer fill size, i.e. <BufferFilledSizeLocation>, set buffer location after the first 64 bits */
    soBufferView_.BufferLocation            = GetNative()->GetGPUVirtualAddress();
    soBufferView_.SizeInBytes               = GetBufferSize();
    soBufferView_.BufferFilledSizeLocation  = GetNative()->GetGPUVirtualAddress() + GetBufferSize();
}

void D3D12Buffer::ClearSubresourceWithUAV(
    ID3D12GraphicsCommandList*  commandList,
    ID3D12Resource*             resource,
    UINT64                      resourceSize,
    D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle,
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle,
    UINT64                      offset,
    UINT64                      fillSize,
    UINT                        formatStride,
    const UINT                  (&valuesVec4)[4])
{
    if (offset == 0 && fillSize == resourceSize)
    {
        /* Fill whole buffer (don't use D3D12_RECT) */
        commandList->ClearUnorderedAccessViewUint(
            gpuDescHandle,
            cpuDescHandle,
            resource,
            valuesVec4,
            0,
            nullptr
        );
    }
    else
    {
        /*
        Fill range of buffer (use D3D12_RECT) and divide by 'formatStride'
        to select structured elements (i.e. D3D12_BUFFER_UAV::NumElements).
        */
        const D3D12_RECT rect =
        {
            static_cast<LONG>((offset           ) / formatStride), 0,
            static_cast<LONG>((offset + fillSize) / formatStride), 1
        };
        commandList->ClearUnorderedAccessViewUint(
            gpuDescHandle,
            cpuDescHandle,
            resource,
            valuesVec4,
            1,
            &rect
        );
    }
}

UINT D3D12Buffer::GetStrideForView(const Format format) const
{
    if (format != Format::Undefined)
    {
        /* Ignore format block size here, we only consider the entire chunk of a format entry */
        const auto& formatAttrib = GetFormatAttribs(format);
        return (formatAttrib.bitSize / 8);
    }
    return stride_;
}


} // /namespace LLGL



// ================================================================================
