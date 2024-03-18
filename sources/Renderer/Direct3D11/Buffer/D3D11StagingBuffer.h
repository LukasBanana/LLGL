/*
 * D3D11StagingBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_STAGING_BUFFER_H
#define LLGL_D3D11_STAGING_BUFFER_H


#include <LLGL/Buffer.h>
#include "../../DXCommon/ComPtr.h"
#include <d3d11.h>


namespace LLGL
{


// Intermediate buffer with a writing offset to be used for both immediate and deferred contexts.
class D3D11StagingBuffer
{

    public:

        D3D11StagingBuffer(
            ID3D11Device*   device,
            UINT            size,
            D3D11_USAGE     usage           = D3D11_USAGE_STAGING,
            UINT            cpuAccessFlags  = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ,
            UINT            bindFlags       = 0
        );

        D3D11StagingBuffer(D3D11StagingBuffer&& rhs) noexcept;
        D3D11StagingBuffer& operator = (D3D11StagingBuffer&& rhs) noexcept;

        D3D11StagingBuffer(const D3D11StagingBuffer&) = delete;
        D3D11StagingBuffer& operator = (const D3D11StagingBuffer&) = delete;

        // Resets the writing offset.
        void Reset();

        // Returns true if the remaining buffer size can fit the specified data size.
        bool Capacity(UINT dataSize) const;

        // Writes the specified data to the native D3D intermediate buffer at the current offset.
        void Write(
            ID3D11DeviceContext*    context,
            const void*             data,
            UINT                    dataSize
        );

        // Writes the specified data to the native D3D intermediate buffer and increments the write offset.
        void WriteAndIncrementOffset(
            ID3D11DeviceContext*    context,
            const void*             data,
            UINT                    dataSize,
            UINT                    stride = 0
        );

        // Returns the native ID3D11Buffer object.
        inline ID3D11Buffer* GetNative() const
        {
            return native_.Get();
        }

        // Returns the buffer size (in bytes).
        inline UINT GetSize() const
        {
            return size_;
        }

        // Returns the current writing offset.
        inline UINT GetOffset() const
        {
            return offset_;
        }

    private:

        ComPtr<ID3D11Buffer>    native_;
        D3D11_USAGE             usage_      = D3D11_USAGE_STAGING;
        UINT                    size_       = 0;
        UINT                    offset_     = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
