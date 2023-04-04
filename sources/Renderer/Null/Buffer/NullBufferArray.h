/*
 * NullBufferArray.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_NULL_BUFFER_ARRAY_H
#define LLGL_NULL_BUFFER_ARRAY_H


#include <LLGL/BufferArray.h>
#include <vector>


namespace LLGL
{


class Buffer;
class NullBuffer;

class NullBufferArray final : public BufferArray
{

    public:

        NullBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray);

    public:

        const std::vector<NullBuffer*> buffers;

};


} // /namespace LLGL


#endif



// ================================================================================
