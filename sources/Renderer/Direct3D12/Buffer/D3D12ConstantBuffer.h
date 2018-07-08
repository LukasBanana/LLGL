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


class D3D12ConstantBuffer final : public D3D12Buffer
{

    public:

        D3D12ConstantBuffer(ID3D12Device* device, const BufferDescriptor& desc);

        void UpdateSubresource(const void* data, UINT bufferSize, UINT64 offset = 0);

        void CreateResourceView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle);

    private:

        void CreateResourceWithAlignment(ID3D12Device* device, UINT bufferSize);

        UINT bufferSize_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
