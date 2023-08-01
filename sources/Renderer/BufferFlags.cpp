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
        desc.stride == 0 &&
        desc.format != Format::Undefined &&
        (desc.bindFlags & (BindFlags::Sampled | BindFlags::Storage)) != 0
    );
}

LLGL_EXPORT bool IsStructuredBuffer(const BufferDescriptor& desc)
{
    return
    (
        desc.stride > 0 &&
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
