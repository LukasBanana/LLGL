/*
 * D3D11StagingBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11StagingBuffer.h"
#include "../D3D11ResourceFlags.h"
#include "../../DXCommon/DXCore.h"
#include "../../../Core/Assertion.h"
#include <algorithm>


namespace LLGL
{


static UINT DXGetAllowedCPUAccessFlags(D3D11_USAGE usage)
{
    if (usage == D3D11_USAGE_DYNAMIC)
        return D3D11_CPU_ACCESS_WRITE;
    else if (usage == D3D11_USAGE_STAGING)
        return D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
    else
        return 0;
}

D3D11StagingBuffer::D3D11StagingBuffer(
    ID3D11Device*   device,
    UINT            size,
    D3D11_USAGE     usage,
    UINT            cpuAccessFlags,
    UINT            bindFlags)
:
    usage_ { usage },
    size_  { size  }
{
    /* Create new D3D11 hardware buffer (for CPU access) */
    LLGL_ASSERT((cpuAccessFlags & DXGetAllowedCPUAccessFlags(usage)) == cpuAccessFlags, "invalid CPU-access flags for D3D11 buffer usage");
    D3D11_BUFFER_DESC descD3D;
    {
        descD3D.ByteWidth           = size;
        descD3D.Usage               = usage;
        descD3D.BindFlags           = bindFlags;
        descD3D.CPUAccessFlags      = cpuAccessFlags;
        descD3D.MiscFlags           = 0; // MiscFlags cannot be used for buffers with D3D11_CPU_ACCESS flags
        descD3D.StructureByteStride = 0;
    }
    HRESULT hr = device->CreateBuffer(&descD3D, nullptr, native_.GetAddressOf());
    DXThrowIfCreateFailed(hr, "ID3D11Buffer", "for CPU-access buffer");
}

D3D11StagingBuffer::D3D11StagingBuffer(D3D11StagingBuffer&& rhs) noexcept :
    native_ { std::move(rhs.native_) },
    usage_  { rhs.usage_             },
    size_   { rhs.size_              },
    offset_ { rhs.offset_            }
{
}

D3D11StagingBuffer& D3D11StagingBuffer::operator = (D3D11StagingBuffer&& rhs) noexcept
{
    if (this != &rhs)
    {
        native_ = std::move(rhs.native_);
        usage_  = rhs.usage_;
        size_   = rhs.size_;
        offset_ = rhs.offset_;
    }
    return *this;
}

void D3D11StagingBuffer::Reset()
{
    offset_ = 0;
}

bool D3D11StagingBuffer::Capacity(UINT dataSize) const
{
    return (offset_ + dataSize <= size_);
}

void D3D11StagingBuffer::Write(
    ID3D11DeviceContext*    context,
    const void*             data,
    UINT                    dataSize)
{
    if (usage_ == D3D11_USAGE_DYNAMIC)
    {
        /*
        D3D11_USAGE_DYNAMIC only supports map-write with discard;
        Update partial subresource by mapping buffer from GPU into CPU memory space.
        */
        D3D11_MAPPED_SUBRESOURCE subresource;
        if (SUCCEEDED(context->Map(GetNative(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource)))
        {
            ::memcpy(reinterpret_cast<char*>(subresource.pData) + offset_, data, dataSize);
            context->Unmap(GetNative(), 0);
        }
    }
    else
    {
        /* Update sub region of buffer */
        const D3D11_BOX dstBox{ offset_, 0, 0, offset_ + dataSize, 1, 1 };
        context->UpdateSubresource(GetNative(), 0, &dstBox, data, 0, 0);
    }
}

void D3D11StagingBuffer::WriteAndIncrementOffset(
    ID3D11DeviceContext*    context,
    const void*             data,
    UINT                    dataSize,
    UINT                    stride)
{
    Write(context, data, dataSize);
    offset_ += std::max(dataSize, stride);
}


} // /namespace LLGL



// ================================================================================
