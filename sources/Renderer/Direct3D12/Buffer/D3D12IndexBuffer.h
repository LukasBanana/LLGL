/*
 * D3D12IndexBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D12_INDEX_BUFFER_H__
#define __LLGL_D3D12_INDEX_BUFFER_H__


#include <LLGL/IndexBuffer.h>
#include "D3D12HardwareBuffer.h"


namespace LLGL
{


class D3D12IndexBuffer : public IndexBuffer
{

    public:

        void UpdateSubResource(
            ID3D12Device* device, ID3D12GraphicsCommandList* gfxCommandList, ComPtr<ID3D12Resource>& bufferUpload,
            const void* data, UINT bufferSize, UINT64 offset = 0
        );

        D3D12HardwareBuffer hwBuffer;

};


} // /namespace LLGL


#endif



// ================================================================================
