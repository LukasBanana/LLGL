/*
 * D3D11Buffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_BUFFER_H
#define LLGL_D3D11_BUFFER_H


#include <LLGL/Buffer.h>
#include "../../DXCommon/ComPtr.h"
#include <d3d11.h>


namespace LLGL
{


class D3D11Buffer : public Buffer
{

    public:

        D3D11Buffer(const BufferType type);
        D3D11Buffer(const BufferType type, ID3D11Device* device, const D3D11_BUFFER_DESC& desc, const void* initialData = nullptr, long bufferFlags = 0);

        virtual void UpdateSubresource(ID3D11DeviceContext* context, const void* data, UINT dataSize, UINT offset);
        virtual void UpdateSubresource(ID3D11DeviceContext* context, const void* data);

        void* Map(ID3D11DeviceContext* context, const BufferCPUAccess access);
        void Unmap(ID3D11DeviceContext* context, const BufferCPUAccess access);

        //! Returns the ID3D11Buffer object.
        inline ID3D11Buffer* Get() const
        {
            return buffer_.Get();
        }

    protected:

        void CreateResource(ID3D11Device* device, const D3D11_BUFFER_DESC& desc, const void* initialData, long bufferFlags);

    private:

        void CreateCPUAccessBuffer(ID3D11Device* device, const D3D11_BUFFER_DESC& gpuBufferDesc, UINT cpuAccessFlags);

        ComPtr<ID3D11Buffer> buffer_;
        ComPtr<ID3D11Buffer> cpuAccessBuffer_;

};


} // /namespace LLGL


#endif



// ================================================================================
