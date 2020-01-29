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
#include "../../BufferUtils.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/Helper.h"
#include <stdexcept>


namespace LLGL
{


// BufferFilledSize actually only needs 4 bytes, but we keep it 16 byte aligned
// see https://docs.microsoft.com/en-us/windows/win32/direct3d12/stream-output-counters#bufferfilledsize
static const UINT64 g_soBufferFillSizeLen   = sizeof(UINT64);
static const UINT64 g_cBufferAlignment      = 256u;

// Returns DXGI_FORMAT_UNKNOWN for a structured buffer, or maps the format attribute to DXGI_FORMAT enum.
static DXGI_FORMAT GetDXFormatForBuffer(const BufferDescriptor& desc)
{
    return (IsStructuredBuffer(desc) ? DXGI_FORMAT_UNKNOWN : D3D12Types::Map(desc.format));
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

    /* Create CPU access buffer */
    if (desc.cpuAccessFlags != 0)
        CreateCpuAccessBuffer(device, desc.cpuAccessFlags);

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

    CreateShaderResourceViewPrimary(device, cpuDescHandle, firstElement, numElements, stride, D3D12Types::Map(bufferViewDesc.format));
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

    CreateUnorderedAccessViewPrimary(device, cpuDescHandle, firstElement, numElements, stride, D3D12Types::Map(bufferViewDesc.format));
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

static bool HasReadAccess(const CPUAccess access)
{
    return (access == CPUAccess::ReadOnly || access == CPUAccess::ReadWrite);
}

static bool HasWriteAccess(const CPUAccess access)
{
    return (access >= CPUAccess::WriteOnly && access <= CPUAccess::ReadWrite);
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

    /* Create intermediate descritpor heap if not already done or format changed */
    if (!uavIntermediateDescHeap_ || format_ != format)
        CreateIntermediateUAVDescriptorHeap(resource, format, formatStride);

    /* Get GPU and CPU descriptor handles for intermediate descriptor heap */
    D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle = uavIntermediateDescHeap_->GetGPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle = uavIntermediateDescHeap_->GetCPUDescriptorHandleForHeapStart();

    if (useIntermediateBuffer)
    {
        /* Clear interemdiate buffer with UAV */
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
        {
            if (fillSize == GetBufferSize())
                commandList->CopyResource(GetNative(), uavIntermediateBuffer_.Get());
            else
                commandList->CopyBufferRegion(GetNative(), offset, uavIntermediateBuffer_.Get(), offset, fillSize);
        }
        commandContext.TransitionResource(uavIntermediateBuffer_, uavIntermediateBuffer_.usageState);
        commandContext.TransitionResource(GetResource(), GetResource().usageState, true);
    }
    else
    {
        /* Clear destination buffer directly with intermediate UAV */
        commandContext.TransitionResource(GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
        {
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
        commandContext.TransitionResource(GetResource(), GetResource().usageState, true);
    }
}

HRESULT D3D12Buffer::Map(
    D3D12CommandContext&    commandContext,
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
                commandContext.GetCommandList()->CopyBufferRegion(
                    cpuAccessBuffer_.Get(),
                    range.Begin,
                    GetNative(),
                    range.Begin,
                    range.End - range.Begin
                );
            }
            commandContext.TransitionResource(resource_, resource_.usageState, true);
            commandContext.Finish(true);

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

void D3D12Buffer::Unmap(D3D12CommandContext& commandContext)
{
    if (cpuAccessBuffer_.Get() != nullptr)
    {
        if (HasWriteAccess(mappedCPUaccess_))
        {
            /* Unmap with written range */
            cpuAccessBuffer_.Get()->Unmap(0, &mappedRange_);

            /* Copy content from CPU memory to GPU host memory */
            commandContext.TransitionResource(resource_, D3D12_RESOURCE_STATE_COPY_DEST, true);
            {
                commandContext.GetCommandList()->CopyBufferRegion(
                    GetNative(),
                    mappedRange_.Begin,
                    cpuAccessBuffer_.Get(),
                    mappedRange_.Begin,
                    mappedRange_.End - mappedRange_.Begin
                );
            }
            commandContext.TransitionResource(resource_, resource_.usageState, true);
            commandContext.Finish(true);
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

//TODO: transition sources before binding
static D3D12_RESOURCE_STATES GetD3DUsageState(long bindFlags)
{
    D3D12_RESOURCE_STATES flagsD3D = D3D12_RESOURCE_STATE_COMMON;

    if ((bindFlags & (BindFlags::VertexBuffer | BindFlags::ConstantBuffer)) != 0)
        flagsD3D |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    else if ((bindFlags & BindFlags::IndexBuffer) != 0)
        flagsD3D |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
    else if ((bindFlags & BindFlags::Storage) != 0)
        flagsD3D |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    else if ((bindFlags & BindFlags::StreamOutputBuffer) != 0)
        flagsD3D |= D3D12_RESOURCE_STATE_STREAM_OUT;
    else if ((bindFlags & BindFlags::IndirectBuffer) != 0)
        flagsD3D |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;

    //if ((bindFlags & BindFlags::Sampled) != 0)
    //    flagsD3D |= (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    return flagsD3D;
}

// see https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/nf-d3d12-id3d12device-createcommittedresource
void D3D12Buffer::CreateGpuBuffer(ID3D12Device* device, const BufferDescriptor& desc)
{
    /* Store buffer attributes */
    bufferSize_ = GetAlignedSize<UINT64>(desc.size, alignment_);
    stride_     = GetStorageBufferStride(desc);

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

void D3D12Buffer::CreateCpuAccessBuffer(ID3D12Device* device, long cpuAccessFlags)
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

    /* Store new format */
    format_ = format;
}

void D3D12Buffer::CreateIntermediateUAVBuffer()
{
    /* Use device the resource was created with */
    ComPtr<ID3D12Device> device;
    resource_.Get()->GetDevice(IID_PPV_ARGS(&device));

    /* Create intermediate resource with UAV support */
    auto hr = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(GetInternalBufferSize(), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        nullptr,
        IID_PPV_ARGS(uavIntermediateBuffer_.native.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", "for buffer subresource UAV");

    uavIntermediateBuffer_.SetInitialState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
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
    indexBufferView_.Format         = D3D12Types::Map(desc.format);
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
        Fill range of buffer (use D3D12_RECT) and devide by 'formatStride'
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
