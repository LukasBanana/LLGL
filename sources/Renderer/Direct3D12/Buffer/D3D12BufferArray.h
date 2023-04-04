/*
 * D3D12BufferArray.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_BUFFER_ARRAY_H
#define LLGL_D3D12_BUFFER_ARRAY_H


#include <LLGL/BufferArray.h>
#include <d3d12.h>
#include <vector>


namespace LLGL
{


class Buffer;

class D3D12BufferArray final : public BufferArray
{

    public:

        D3D12BufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray);

        // Returns the array of vertex buffer views.
        inline const std::vector<D3D12_VERTEX_BUFFER_VIEW>& GetVertexBufferViews() const
        {
            return vertexBufferViews_;
        }

    private:

        std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViews_;

};


} // /namespace LLGL


#endif



// ================================================================================
