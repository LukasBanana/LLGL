/*
 * D3D11ConstantBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_CONSTANT_BUFFER_H
#define LLGL_D3D11_CONSTANT_BUFFER_H


#include "D3D11Buffer.h"


namespace LLGL
{


class D3D11ConstantBuffer final : public D3D11Buffer
{

    public:

        D3D11ConstantBuffer(ID3D11Device* device, const BufferDescriptor& desc, const void* initialData = nullptr);

        void UpdateSubresource(ID3D11DeviceContext* context, const void* data, UINT dataSize, UINT offset) override;

    private:

        UINT        bufferSize_ = 0;
        D3D11_USAGE usage_      = D3D11_USAGE_DEFAULT;

};


} // /namespace LLGL


#endif



// ================================================================================
