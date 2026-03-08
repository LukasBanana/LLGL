/*
 * D3D9IndexBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9IndexBuffer.h"
#include "../../ResourceUtils.h"
#include "../../../Core/CoreUtils.h"
#include "../D3D9Core.h"
#include "../D3D9Types.h"


namespace LLGL
{


D3D9IndexBuffer::D3D9IndexBuffer(IDirect3DDevice9* device, const BufferDescriptor& desc) :
    D3D9Buffer { desc.bindFlags }
{
    const UINT bufferLength = static_cast<UINT>(desc.size);
    const DWORD usageFlags = D3D9Buffer::GetUsageFlags(desc.bindFlags, desc.cpuAccessFlags, desc.miscFlags);

    HRESULT hr = device->CreateIndexBuffer(
        bufferLength,
        usageFlags, // Usage
        D3D9Types::ToD3DIndexFormat(desc.format),
        D3DPOOL_DEFAULT,
        d3dBuffer_.GetAddressOf(),
        nullptr // Shared handle (must be null)
    );
    D3DThrowIfCreateFailed(hr, "IDirect3DIndexBuffer9");
}

bool D3D9IndexBuffer::GetNativeHandle(void* /*nativeHandle*/, std::size_t /*nativeHandleSize*/)
{
    return false; // dummy
}

BufferDescriptor D3D9IndexBuffer::GetDesc() const
{
    BufferDescriptor outDesc;
    {
        D3DINDEXBUFFER_DESC d3dDesc;
        d3dBuffer_->GetDesc(&d3dDesc);

        outDesc.size = d3dDesc.Size;
        outDesc.format = D3D9Types::ToFormat(d3dDesc.Format);

        /* Ingore internally stored binding flags; The D3D9 backend can only bind this buffer as a index buffer. */
        outDesc.bindFlags = BindFlags::IndexBuffer;
    }
    return outDesc;
}

HRESULT D3D9IndexBuffer::Write(UINT dstOffset, const void* data, UINT dataSize)
{
    return D3D9Buffer::WriteLocked(d3dBuffer_.Get(), dstOffset, data, dataSize);
}


} // /namespace LLGL



// ================================================================================
