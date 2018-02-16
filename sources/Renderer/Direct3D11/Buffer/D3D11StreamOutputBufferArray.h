/*
 * D3D11StreamOutputBufferArray.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_STREAM_OUTPUT_BUFFER_ARRAY_H
#define LLGL_D3D11_STREAM_OUTPUT_BUFFER_ARRAY_H


#include "D3D11BufferArray.h"


namespace LLGL
{


class D3D11StreamOutputBufferArray : public D3D11BufferArray
{

    public:

        D3D11StreamOutputBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray);

        // Returns the array of buffer offsets.
        inline const std::vector<UINT>& GetOffsets() const
        {
            return offsets_;
        }

    private:

        std::vector<UINT> offsets_;

};


} // /namespace LLGL


#endif



// ================================================================================
