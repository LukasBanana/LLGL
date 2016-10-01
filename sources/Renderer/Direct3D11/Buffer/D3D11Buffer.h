/*
 * D3D11Buffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D11_BUFFER_H__
#define __LLGL_D3D11_BUFFER_H__


#include <LLGL/Buffer.h>
#include "../../ComPtr.h"
#include <d3d11.h>


namespace LLGL
{


class D3D11Buffer : public Buffer
{

    public:

        D3D11Buffer(const BufferType type);
        D3D11Buffer(const BufferType type, ID3D11Device* device, const D3D11_BUFFER_DESC& desc, const void* initialData = nullptr);

        virtual void UpdateSubresource(ID3D11DeviceContext* context, const void* data, UINT dataSize, UINT offset);
        virtual void UpdateSubresource(ID3D11DeviceContext* context, const void* data);

        //! Returns the ID3D11Buffer object.
        inline ID3D11Buffer* Get() const
        {
            return buffer_.Get();
        }

    protected:

        void CreateResource(ID3D11Device* device, const D3D11_BUFFER_DESC& desc, const void* initialData = nullptr);

    private:

        ComPtr<ID3D11Buffer> buffer_;

};


} // /namespace LLGL


#endif



// ================================================================================
