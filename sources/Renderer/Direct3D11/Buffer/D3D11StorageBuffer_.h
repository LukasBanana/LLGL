/*
 * D3D11StorageBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D11_STORAGE_BUFFER_H__
#define __LLGL_D3D11_STORAGE_BUFFER_H__


#include "D3D11Buffer.h"


namespace LLGL
{


class D3D11StorageBuffer_ : public D3D11Buffer
{

    public:

        D3D11StorageBuffer_(ID3D11Device* device, const BufferDescriptor& desc, const void* initialData = nullptr);

        bool IsUAV() const;
        bool IsStructured() const;
        bool IsByteAddressable() const;

        inline ID3D11ShaderResourceView* GetSRV() const
        {
            return srv_.Get();
        }

        inline ID3D11UnorderedAccessView* GetUAV() const
        {
            return uav_.Get();
        }

    private:

        void CreateUAV(ID3D11Device* device);
        void CreateSRV(ID3D11Device* device);

        StorageBufferType                   storageType_ = StorageBufferType::Buffer;

        ComPtr<ID3D11ShaderResourceView>    srv_;
        ComPtr<ID3D11UnorderedAccessView>   uav_;

};


} // /namespace LLGL


#endif



// ================================================================================
