/*
 * D3D11VertexBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D11_VERTEX_BUFFER_H__
#define __LLGL_D3D11_VERTEX_BUFFER_H__


#include "D3D11Buffer.h"


namespace LLGL
{


class D3D11VertexBuffer : public D3D11Buffer
{

    public:

        D3D11VertexBuffer(ID3D11Device* device, const BufferDescriptor& desc, const void* initialData = nullptr);

        inline UINT GetStride() const
        {
            return stride_;
        }

    private:

        UINT stride_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
