/*
 * D3D12IndexBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D12_INDEX_BUFFER_H__
#define __LLGL_D3D12_INDEX_BUFFER_H__


#include "D3D12Buffer.h"


namespace LLGL
{


class D3D12IndexBuffer : public D3D12Buffer
{

    public:

        D3D12IndexBuffer(ID3D12Device* device, const BufferDescriptor& desc);

        void UpdateSubresource(
            ID3D12Device* device, ID3D12GraphicsCommandList* gfxCommandList, ComPtr<ID3D12Resource>& bufferUpload,
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
