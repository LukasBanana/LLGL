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
#include "../Direct3D9.h"


namespace LLGL
{


class Buffer;
class D3D9VertexBuffer;

class D3D9BufferArray final : public BufferArray
{

    public:

        struct D3DBufferAndStride
        {
            IDirect3DVertexBuffer9* vertexBuffer;
            UINT                    stride;
        };

    public:

        D3D9BufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray);

        inline const std::vector<D3DBufferAndStride>& GetNativeBuffersAndStrides() const
        {
            return nativeBuffersAndStrides_;
        }

    private:

        std::vector<D3DBufferAndStride> nativeBuffersAndStrides_;

};


} // /namespace LLGL


#endif



// ================================================================================
