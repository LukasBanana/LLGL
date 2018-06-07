/*
 * D3D12ConstantBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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

        void CreateResourceView(ID3D12Device* device, ID3D12DescriptorHeap* descriptorHeap);

    private:

        void CreateResourceWithAlignment(ID3D12Device* device, UINT bufferSize);

        UINT bufferSize_ = 0;

        //ComPtr<ID3D12DescriptorHeap> descHeap_; //TODO: replace this by D3D12ResourceHeap

};


} // /namespace LLGL


#endif



// ================================================================================
