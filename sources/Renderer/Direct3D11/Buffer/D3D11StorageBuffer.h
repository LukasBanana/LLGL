/*
 * D3D11StorageBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_STORAGE_BUFFER_H
#define LLGL_D3D11_STORAGE_BUFFER_H


#include "D3D11Buffer.h"


namespace LLGL
{


class D3D11StorageBuffer : public D3D11Buffer
{

    public:

        D3D11StorageBuffer(ID3D11Device* device, const BufferDescriptor& desc, const void* initialData = nullptr);

        // True, if this storage buffer has a UAV object.
        bool HasUAV() const;

        // True, if storage type is: Buffer or RWBuffer.
        bool IsTyped() const;

        // True, if storage type is: StructuredBuffer, RWStructuredBuffer, AppendStructuredBuffer, or ConsumeStructuredBuffer.
        bool IsStructured() const;

        // True, if storage type is: ByteAddressBuffer or RWByteAddressBuffer.
        bool IsByteAddressable() const;

        inline ID3D11ShaderResourceView* GetSRV() const
        {
            return srv_.Get();
        }

        inline ID3D11UnorderedAccessView* GetUAV() const
        {
            return uav_.Get();
        }

        inline UINT GetInitialCount() const
        {
            return initialCount_;
        }

    private:

        UINT GetBindFlags() const;
        UINT GetMiscFlags() const;
        UINT GetUAVFlags() const;

        DXGI_FORMAT GetFormat(const VectorType vectorType) const;

        void CreateSRV(ID3D11Device* device, DXGI_FORMAT format, UINT firstElement, UINT numElements);
        void CreateUAV(ID3D11Device* device, DXGI_FORMAT format, UINT firstElement, UINT numElements);

        StorageBufferType                   storageType_        = StorageBufferType::Buffer;

        ComPtr<ID3D11ShaderResourceView>    srv_;
        ComPtr<ID3D11UnorderedAccessView>   uav_;

        UINT                                initialCount_       = -1;

};


} // /namespace LLGL


#endif



// ================================================================================
