/*
 * D3D11ConstantBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D11_CONSTANT_BUFFER__DEPRECATED__H__
#define __LLGL_D3D11_CONSTANT_BUFFER__DEPRECATED__H__


#include <LLGL/ConstantBuffer.h>
#include <LLGL/RenderSystemFlags.h>
#include "D3D11HardwareBuffer.h"


namespace LLGL
{


class D3D11ConstantBuffer : public ConstantBuffer
{

    public:

        void CreateResource(ID3D11Device* device, const ConstantBufferDescriptor& desc, const void* initialData = nullptr);

        void UpdateSubresource(ID3D11DeviceContext* context, const void* data, UINT dataSize, UINT offset);

        D3D11HardwareBuffer hwBuffer;

    private:

        UINT        bufferSize_ = 0;
        D3D11_USAGE usage_      = D3D11_USAGE_DEFAULT;

};


} // /namespace LLGL


#endif



// ================================================================================
