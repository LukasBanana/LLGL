/*
 * D3D12BufferWithCPUAccess.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12BufferWithCPUAccess.h"
#include "../D3DX12/d3dx12.h"
#include "../D3D12Types.h"
#include "../D3D12ObjectUtils.h"
#include "../../DXCommon/DXCore.h"
#include "../../../Core/CoreUtils.h"
#include <string.h>


namespace LLGL
{


D3D12BufferWithCPUAccess::D3D12BufferWithCPUAccess(ID3D12Device* device, const BufferDescriptor& desc) :
    D3D12Buffer { device, desc }
{
    if ((desc.cpuAccessFlags & CPUAccessFlags::Read) != 0)
        CreateNativeCPUAccessBuffer(device, readbackBuffer_, D3D12_HEAP_TYPE_READBACK);
    if ((desc.cpuAccessFlags & CPUAccessFlags::Write) != 0)
        CreateNativeCPUAccessBuffer(device, uploadBuffer_, D3D12_HEAP_TYPE_UPLOAD);
}

void D3D12BufferWithCPUAccess::SetDebugName(const char* name)
{
    D3D12Buffer::SetDebugName(name);

    if (uploadBuffer_.Get() != nullptr)
        D3D12SetObjectNameSubscript(uploadBuffer_.Get(), name, ".Upload");
    if (readbackBuffer_.Get() != nullptr)
        D3D12SetObjectNameSubscript(readbackBuffer_.Get(), name, ".Readback");
}

void* D3D12BufferWithCPUAccess::Map(CPUAccess access, std::uint64_t offset, std::uint64_t length)
{
    if (access == CPUAccess::ReadWrite)
    {
        if (ID3D12Resource* uploadResource = uploadBuffer_.Get())
        {
            if (ID3D12Resource* readbackResource = readbackBuffer_.Get())
            {
                writeRange_.Begin   = offset;
                writeRange_.End     = offset + length;

                void* uploadData = nullptr;
                const D3D12_RANGE nullRange{ 0, 0, };
                HRESULT hr = uploadResource->Map(0, &nullRange, &uploadData);
                if (SUCCEEDED(hr))
                {
                    void* readbackData = nullptr;
                    HRESULT hr = readbackResource->Map(0, &writeRange_, &readbackData);
                    if (SUCCEEDED(hr))
                    {
                        /* Copy readback data into upload buffer before returning the mapped data */
                        ::memcpy(uploadData, static_cast<char*>(readbackData) + offset, length);
                        readbackResource->Unmap(0, &nullRange);
                        return uploadData;
                    }
                }
            }
        }
    }
    else if (access == CPUAccess::ReadOnly)
    {
        if (ID3D12Resource* readbackResource = readbackBuffer_.Get())
        {
            void* data = nullptr;
            const D3D12_RANGE mapReadRange{ offset, offset + length };
            HRESULT hr = readbackResource->Map(0, &mapReadRange, &data);
            if (SUCCEEDED(hr))
                return (static_cast<char*>(data) + offset);
        }
    }
    else
    {
        if (ID3D12Resource* uploadResource = uploadBuffer_.Get())
        {
            writeRange_.Begin   = offset;
            writeRange_.End     = offset + length;

            void* data = nullptr;
            const D3D12_RANGE nullRange{ 0, 0 };
            HRESULT hr = uploadResource->Map(0, &nullRange, &data);
            if (SUCCEEDED(hr))
                return (static_cast<char*>(data) + offset);
        }
    }

    return nullptr;
}

void D3D12BufferWithCPUAccess::Unmap()
{
    if (writeRange_.Begin < writeRange_.End)
    {
        if (ID3D12Resource* uploadResource = uploadBuffer_.Get())
        {
            uploadResource->Unmap(0, &writeRange_);
            writeRange_.Begin   = 0;
            writeRange_.End     = 0;
        }
    }
    else
    {
        if (ID3D12Resource* readbackResource = readbackBuffer_.Get())
        {
            const D3D12_RANGE nullRange{ 0, 0 };
            readbackResource->Unmap(0, &nullRange);
        }
    }
}

bool D3D12BufferWithCPUAccess::HasCPUAccess() const
{
    return true;
}

D3D12Resource& D3D12BufferWithCPUAccess::GetResourceForState(D3D12_RESOURCE_STATES state)
{
    if (state == D3D12_RESOURCE_STATE_COPY_DEST)
        return readbackBuffer_;
    if (state == D3D12_RESOURCE_STATE_COPY_SOURCE)
        return uploadBuffer_;
    return D3D12Buffer::GetResourceForState(state);
}


/*
 * ======= Private: =======
 */

void D3D12BufferWithCPUAccess::CreateNativeCPUAccessBuffer(ID3D12Device* device, D3D12Resource& resource, D3D12_HEAP_TYPE heapType)
{
    /* Create GPU upload buffer */
    const CD3DX12_HEAP_PROPERTIES heapProperties{ heapType };
    const CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(GetBufferSize());
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        (heapType == D3D12_HEAP_TYPE_READBACK ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_GENERIC_READ),
        nullptr,
        IID_PPV_ARGS(resource.native.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", heapType == D3D12_HEAP_TYPE_READBACK ? "for readback buffer" : "for upload buffer");
}


} // /namespace LLGL



// ================================================================================
