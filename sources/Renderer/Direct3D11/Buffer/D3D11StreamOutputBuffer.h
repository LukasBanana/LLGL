/*
 * D3D11StreamOutputBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_STREAM_OUTPUT_BUFFER_H
#define LLGL_D3D11_STREAM_OUTPUT_BUFFER_H


#include "D3D11Buffer.h"


namespace LLGL
{


class D3D11StreamOutputBuffer final : public D3D11Buffer
{

    public:

        D3D11StreamOutputBuffer(ID3D11Device* device, const BufferDescriptor& desc, const void* initialData = nullptr);

        // Returns the offset of this stream-output buffer.
        inline UINT GetOffset() const
        {
            return offset_;
        }

    private:

        UINT offset_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
