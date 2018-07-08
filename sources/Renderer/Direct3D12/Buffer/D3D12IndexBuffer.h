/*
 * D3D12IndexBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_INDEX_BUFFER_H
#define LLGL_D3D12_INDEX_BUFFER_H


#include "D3D12Buffer.h"


namespace LLGL
{


class D3D12IndexBuffer final : public D3D12Buffer
{

    public:

        D3D12IndexBuffer(ID3D12Device* device, const BufferDescriptor& desc);

        void UpdateSubresource(
            ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ComPtr<ID3D12Resource>& uploadBuffer,
            const void* data, UINT bufferSize, UINT64 offset = 0
        );

        inline const D3D12_INDEX_BUFFER_VIEW& GetView() const
        {
            return view_;
        }

    private:

        D3D12_INDEX_BUFFER_VIEW view_;

};


} // /namespace LLGL


#endif



// ================================================================================
