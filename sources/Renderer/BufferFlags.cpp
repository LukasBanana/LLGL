/*
 * BufferFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
