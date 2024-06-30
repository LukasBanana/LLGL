/*
 * D3D12BufferArray.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_BUFFER_ARRAY_H
#define LLGL_D3D12_BUFFER_ARRAY_H


#include <LLGL/BufferArray.h>
#include <LLGL/Container/SmallVector.h>
#include <d3d12.h>


namespace LLGL
{


class Buffer;
struct D3D12Resource;

class D3D12BufferArray final : public BufferArray
{

    public:

        D3D12BufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray);

        // Returns the array of vertex buffer views.
        inline ArrayView<D3D12_VERTEX_BUFFER_VIEW> GetVertexBufferViews() const
        {
            return vertexBufferViews_;
        }

        // Returns the array of vertex buffer resource references.
        inline ArrayView<D3D12Resource*> GetResourceRefs() const
        {
            return resourceRefs_;
        }

    private:

        SmallVector<D3D12_VERTEX_BUFFER_VIEW, 4>    vertexBufferViews_;
        SmallVector<D3D12Resource*, 4>              resourceRefs_;

};


} // /namespace LLGL


#endif



// ================================================================================
