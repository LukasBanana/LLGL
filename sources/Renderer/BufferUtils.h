/*
 * BufferUtils.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_BUFFER_UTILS_H
#define LLGL_BUFFER_UTILS_H


#include <LLGL/BufferFlags.h>


namespace LLGL
{


/* ----- Functions ----- */

/*
Returns the final stride (in bytes) for a storage buffer, i.e. either by <stride> attribute of <format>.
If <format> is undefined, then 1 is returned for byte address buffers.
*/
LLGL_EXPORT std::uint32_t GetStorageBufferStride(const BufferDescriptor& desc);

// Returns true if the buffer-view in the specified resource-view descriptor is enabled.
inline bool IsBufferViewEnabled(const BufferViewDescriptor& bufferViewDesc)
{
    return
    (
        bufferViewDesc.format != Format::Undefined   ||
        bufferViewDesc.offset != 0                   ||
        bufferViewDesc.size   != Constants::wholeSize
    );
}


} // /namespace LLGL


#endif



// ================================================================================
