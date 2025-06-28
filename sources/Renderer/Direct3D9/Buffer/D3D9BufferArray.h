/*
 * D3D9BufferArray.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_BUFFER_ARRAY_H
#define LLGL_D3D9_BUFFER_ARRAY_H


#include <LLGL/BufferArray.h>
#include <vector>


namespace LLGL
{


class Buffer;
class D3D9VertexBuffer;

class D3D9BufferArray final : public BufferArray
{

    public:

        D3D9BufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray);

    private:

        std::vector<D3D9VertexBuffer*> vertexBuffers_;

};


} // /namespace LLGL


#endif



// ================================================================================
