/*
 * D3D11BufferArray.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_BUFFER_ARRAY_H
#define LLGL_D3D11_BUFFER_ARRAY_H


#include <LLGL/BufferArray.h>
#include <vector>
#include <d3d11.h>


namespace LLGL
{


class Buffer;

class D3D11BufferArray final : public BufferArray
{

    public:

        D3D11BufferArray(long bindFlags, std::uint32_t numBuffers, Buffer* const * bufferArray);

        // Returns the number fo buffers in this array.
        UINT GetCount() const;

        // Returns a pointer to the native buffer objects.
        ID3D11Buffer* const * GetBuffers() const;

        // Returns a pointer to the buffer strides.
        const UINT* GetStrides() const;

        // Returns a pointer to the buffer offsets.
        const UINT* GetOffsets() const;

    private:

        std::vector<ID3D11Buffer*>  buffers_;
        std::vector<UINT>           stridesAndOffsets_;
        std::size_t                 offsetStart_        = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
