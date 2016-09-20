/*
 * D3D11VertexBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D11_VERTEX_BUFFER_H__
#define __LLGL_D3D11_VERTEX_BUFFER_H__


#include <LLGL/VertexBuffer.h>
#include "D3D11HardwareBuffer.h"


namespace LLGL
{


class D3D11VertexBuffer : public VertexBuffer
{

    public:

        void CreateResource(ID3D11Device* device, UINT stride, UINT bufferSize, const void* initialData = nullptr);

        inline UINT GetStride() const
        {
            return stride_;
        }

        D3D11HardwareBuffer hwBuffer;

    private:

        UINT stride_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
