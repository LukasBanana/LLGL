/*
 * D3D11IntermediateBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_INTERMEDIATE_BUFFER_H
#define LLGL_D3D11_INTERMEDIATE_BUFFER_H


#include <LLGL/Buffer.h>
#include "../../DXCommon/ComPtr.h"
#include <d3d11.h>


namespace LLGL
{


// Intermediate buffer with a writing offset to be used for both immediate and deferred contexts.
class D3D11IntermediateBuffer
{

    public:

        D3D11IntermediateBuffer(
            ID3D11Device*   device,
            UINT            size,
            UINT            bindFlags   = 0,
            UINT            miscFlags   = 0
        );

        D3D11IntermediateBuffer(D3D11IntermediateBuffer&& rhs);
        D3D11IntermediateBuffer& operator = (D3D11IntermediateBuffer&& rhs);

        D3D11IntermediateBuffer(const D3D11IntermediateBuffer&) = delete;
        D3D11IntermediateBuffer& operator = (const D3D11IntermediateBuffer&) = delete;

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
            UINT                    dataSize
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
        D3D11_USAGE             usage_  = D3D11_USAGE_DEFAULT;
        UINT                    size_   = 0;
        UINT                    offset_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
