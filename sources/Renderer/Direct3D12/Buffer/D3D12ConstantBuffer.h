/*
 * D3D12ConstantBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_CONSTANT_BUFFER_H
#define LLGL_D3D12_CONSTANT_BUFFER_H


#include "D3D12Buffer.h"


namespace LLGL
{


class D3D12ConstantBuffer : public D3D12Buffer
{

    public:

        D3D12ConstantBuffer(ID3D12Device* device, const BufferDescriptor& desc);

        void UpdateSubresource(const void* data, UINT bufferSize, UINT64 offset = 0);

        inline ID3D12DescriptorHeap* GetDescriptorHeap() const
        {
            return descHeap_.Get();
        }

    private:

        void CreateResourceAndPutView(ID3D12Device* device, UINT bufferSize);

        ComPtr<ID3D12DescriptorHeap> descHeap_; // descriptor heap for constant buffer views (CBV)

};


} // /namespace LLGL


#endif



// ================================================================================
