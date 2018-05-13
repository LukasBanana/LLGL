/*
 * D3D11VertexBufferArray.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_VERTEX_BUFFER_ARRAY_H
#define LLGL_D3D11_VERTEX_BUFFER_ARRAY_H


#include "D3D11BufferArray.h"


namespace LLGL
{


class D3D11VertexBufferArray : public D3D11BufferArray
{

    public:

        D3D11VertexBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray);

        // Returns the array of buffer strides.
        inline const std::vector<UINT>& GetStrides() const
        {
            return strides_;
        }

        // Returns the array of buffer offsets.
        inline const std::vector<UINT>& GetOffsets() const
        {
            return offsets_;
        }

    private:

        std::vector<UINT> strides_;
        std::vector<UINT> offsets_;

};


} // /namespace LLGL


#endif



// ================================================================================
