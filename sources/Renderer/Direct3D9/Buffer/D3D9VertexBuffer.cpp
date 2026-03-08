/*
 * D3D9VertexBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9VertexBuffer.h"
#include "../D3D9Core.h"
#include "../../ResourceUtils.h"
#include "string.h"


namespace LLGL
{


D3D9VertexBuffer::D3D9VertexBuffer(IDirect3DDevice9* device, const BufferDescriptor& desc) :
    D3D9Buffer { desc.bindFlags                                                     },
    stride_    { desc.vertexAttribs.empty() ? 1 : desc.vertexAttribs.front().stride }
{
    const UINT bufferLength = static_cast<UINT>(desc.size);
    const DWORD usageFlags = D3D9Buffer::GetUsageFlags(desc.bindFlags, desc.cpuAccessFlags, desc.miscFlags);

    HRESULT hr = device->CreateVertexBuffer(
        bufferLength,
        usageFlags, // Usage
        0, // Flexible Vertex Format (FVF) not needed since we have a vertex declaration (IDirect3DVertexDeclaration9)
        D3DPOOL_DEFAULT,
        d3dBuffer_.GetAddressOf(),
        nullptr // Shared handle (must be null)
    );
    D3DThrowIfCreateFailed(hr, "IDirect3DVertexBuffer9");
}

bool D3D9VertexBuffer::GetNativeHandle(void* /*nativeHandle*/, std::size_t /*nativeHandleSize*/)
{
    return false; // dummy
}

BufferDescriptor D3D9VertexBuffer::GetDesc() const
{
    BufferDescriptor outDesc;
    {
        D3DVERTEXBUFFER_DESC d3dDesc;
        d3dBuffer_->GetDesc(&d3dDesc);

        outDesc.size = d3dDesc.Size;

        /* Ingore internally stored binding flags; The D3D9 backend can only bind this buffer as a vertex buffer. */
        outDesc.bindFlags = BindFlags::VertexBuffer;
    }
    return outDesc;
}

HRESULT D3D9VertexBuffer::Write(UINT dstOffset, const void* data, UINT dataSize)
{
    return D3D9Buffer::WriteLocked(d3dBuffer_.Get(), dstOffset, data, dataSize);
}


} // /namespace LLGL



// ================================================================================
