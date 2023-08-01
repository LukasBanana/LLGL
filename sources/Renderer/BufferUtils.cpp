/*
 * BufferUtils.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "BufferUtils.h"
#include <LLGL/Format.h>
#include <LLGL/Buffer.h>
#include <algorithm>


namespace LLGL
{


LLGL_EXPORT std::uint32_t GetStorageBufferStride(const BufferDescriptor& desc)
{
    if (desc.stride > 0)
        return desc.stride;
    else if (desc.format != Format::Undefined)
        return std::max(1u, (GetFormatAttribs(desc.format).bitSize / 8u));
    else
        return 1;
}

LLGL_EXPORT long GetCombinedBindFlags(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    long bindFlags = 0;
    for (std::uint32_t i = 0; i < numBuffers; ++i)
        bindFlags |= bufferArray[i]->GetBindFlags();
    return bindFlags;
}


} // /namespace LLGL



// ================================================================================
