/*
 * D3D12HardwareBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D12_HARDWARE_BUFFER_H__
#define __LLGL_D3D12_HARDWARE_BUFFER_H__


#include "../../ComPtr.h"
#include <d3d12.h>


namespace LLGL
{


class D3D12HardwareBuffer
{

    public:

        D3D12HardwareBuffer(const D3D12HardwareBuffer&) = delete;
        D3D12HardwareBuffer& operator = (const D3D12HardwareBuffer&) = delete;

        D3D12HardwareBuffer() = default;

        void CreateResource(ID3D12Device* device, UINT bufferSize, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES resourceState);
        void CreateResource(ID3D12Device* device, UINT bufferSize);

        void UpdateStaticSubresource(
            ID3D12Device* device, ID3D12GraphicsCommandList* gfxCommandList, ComPtr<ID3D12Resource>& bufferUpload,
            const void* data, UINT bufferSize, UINT64 offset, D3D12_RESOURCE_STATES uploadState
        );

        void UpdateDynamicSubresource(const void* data, UINT bufferSize, UINT64 offset);

        //! Returns the ID3D12Resource object.
        inline ID3D12Resource* Get() const
        {
            return resource_.Get();
        }

        //! Returns the size (in bytes) of the hardware buffer.
        inline UINT GetBufferSize() const
        {
            return bufferSize_;
        }

    private:

        ComPtr<ID3D12Resource>  resource_;
        UINT                    bufferSize_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
