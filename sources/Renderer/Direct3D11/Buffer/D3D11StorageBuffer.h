/*
 * D3D11StorageBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_STORAGE_BUFFER_H
#define LLGL_D3D11_STORAGE_BUFFER_H


#include "D3D11Buffer.h"


namespace LLGL
{


class D3D11StorageBuffer final : public D3D11Buffer
{

    public:

        D3D11StorageBuffer(ID3D11Device* device, const BufferDescriptor& desc, const void* initialData = nullptr);

        // Returns the native SRV object.
        inline ID3D11ShaderResourceView* GetSRV() const
        {
            return srv_.Get();
        }

        // Returns the native UAV object.
        inline ID3D11UnorderedAccessView* GetUAV() const
        {
            return uav_.Get();
        }

        // Returns the initial value for the internal buffer counter.
        inline UINT GetInitialCount() const
        {
            return initialCount_;
        }

        // Returns the storage buffer type that was specified when the buffer was created.
        inline StorageBufferType GetStorageType() const
        {
            return storageType_;
        }

    private:

        void CreateSRV(ID3D11Device* device, DXGI_FORMAT format, UINT firstElement, UINT numElements);
        void CreateUAV(ID3D11Device* device, DXGI_FORMAT format, UINT firstElement, UINT numElements, UINT flags);

        StorageBufferType                   storageType_        = StorageBufferType::Buffer;

        ComPtr<ID3D11ShaderResourceView>    srv_;
        ComPtr<ID3D11UnorderedAccessView>   uav_;

        UINT                                initialCount_       = -1;

};


} // /namespace LLGL


#endif



// ================================================================================
