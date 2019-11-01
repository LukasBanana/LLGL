/*
 * BufferUtils.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "BufferUtils.h"
#include <LLGL/Format.h>
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


} // /namespace LLGL



// ================================================================================
