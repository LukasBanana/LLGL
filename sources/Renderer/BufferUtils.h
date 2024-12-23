/*
 * BufferUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_BUFFER_UTILS_H
#define LLGL_BUFFER_UTILS_H


#include <LLGL/BufferFlags.h>


namespace LLGL
{


class Buffer;

/* ----- Functions ----- */

/*
Returns the final stride (in bytes) for a storage buffer, i.e. either by <stride> attribute of <format>.
If <format> is undefined, then 1 is returned for byte address buffers.
*/
LLGL_EXPORT std::uint32_t GetStorageBufferStride(const BufferDescriptor& desc);

// Returns the bitwise OR combined binding flags of the specified array of buffers.
LLGL_EXPORT long GetCombinedBindFlags(std::uint32_t numBuffers, Buffer* const * bufferArray);

// Returns true if the buffer-view in the specified resource-view descriptor is enabled.
inline bool IsBufferViewEnabled(const BufferViewDescriptor& bufferViewDesc)
{
    return
    (
        bufferViewDesc.format != Format::Undefined   ||
        bufferViewDesc.offset != 0                   ||
        bufferViewDesc.size   != LLGL_WHOLE_SIZE
    );
}


} // /namespace LLGL


#endif



// ================================================================================
