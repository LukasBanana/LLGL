/*
 * D3D11IndexBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D11_INDEX_BUFFER_H__
#define __LLGL_D3D11_INDEX_BUFFER_H__


#include <LLGL/IndexBuffer.h>
#include "D3D11HardwareBuffer.h"
#include <dxgiformat.h>


namespace LLGL
{


class D3D11IndexBuffer : public IndexBuffer
{

    public:

        void CreateResource(ID3D11Device* device, DXGI_FORMAT format, UINT bufferSize, const void* initialData = nullptr);

        inline DXGI_FORMAT GetFormat() const
        {
            return format_;
        }

        D3D11HardwareBuffer hwBuffer;

    private:

        DXGI_FORMAT format_ = DXGI_FORMAT_UNKNOWN;

};


} // /namespace LLGL


#endif



// ================================================================================
