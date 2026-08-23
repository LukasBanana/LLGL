/*
 * BufferFlags.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/BufferFlags.h>


namespace LLGL
{


LLGL_EXPORT bool IsTypedBuffer(const BufferDescriptor& desc)
{
    return
    (
        (desc.stride == 0 || ((desc.bindFlags & BindFlags::VertexBuffer) != 0)) &&
        desc.format != Format::Undefined &&
        (desc.bindFlags & (BindFlags::Sampled | BindFlags::Storage)) != 0
    );
}

LLGL_EXPORT bool IsStructuredBuffer(const BufferDescriptor& desc)
{
    /*
    We need to make an exception for vertex buffers here:
    They are mutually exclusive with structured buffers in D3D11,
    so they cannot have such a buffer view and the stride must be interpreted differently.
    Same goes for IsTypedBuffer() above.
    See `GetD3DResourceViewFormat()` in D3D11Buffer.cpp.
    */
    return
    (
        desc.stride > 0 &&
        (desc.bindFlags & BindFlags::VertexBuffer) == 0 &&
        (desc.bindFlags & (BindFlags::Sampled | BindFlags::Storage)) != 0
    );
}

LLGL_EXPORT bool IsByteAddressBuffer(const BufferDescriptor& desc)
{
    return
    (
        desc.stride == 0 &&
        desc.format == Format::Undefined &&
        (desc.bindFlags & (BindFlags::Sampled | BindFlags::Storage)) != 0
    );
}


} // /namespace LLGL



// ================================================================================
