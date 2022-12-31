/*
 * NullBufferArray.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
