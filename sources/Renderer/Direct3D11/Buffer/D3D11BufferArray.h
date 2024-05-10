/*
 * D3D11BufferArray.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_BUFFER_ARRAY_H
#define LLGL_D3D11_BUFFER_ARRAY_H


#include <LLGL/BufferArray.h>
#include <vector>
#include <d3d11.h>


namespace LLGL
{


class Buffer;
struct D3D11BindingLocator;

class D3D11BufferArray final : public BufferArray
{

    public:

        D3D11BufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray);

        // Returns the number fo buffers in this array.
        inline UINT GetCount() const
        {
            return static_cast<UINT>(buffersAndBindingLocators_.size() / 2);
        }

        // Returns a pointer to the native buffer objects.
        inline ID3D11Buffer* const * GetBuffers() const
        {
            return reinterpret_cast<ID3D11Buffer* const *>(buffersAndBindingLocators_.data());
        }

        // Returns a pointer to the binding locators.
        inline D3D11BindingLocator* const * GetBindingLocators() const
        {
            return reinterpret_cast<D3D11BindingLocator* const *>(buffersAndBindingLocators_.data() + GetCount());
        }

        // Returns a pointer to the buffer strides.
        inline const UINT* GetStrides() const
        {
            return stridesAndOffsets_.data();
        }

        // Returns a pointer to the buffer offsets.
        inline const UINT* GetOffsets() const
        {
            return (stridesAndOffsets_.data() + GetCount());
        }

    private:

        std::vector<void*>  buffersAndBindingLocators_;
        std::vector<UINT>   stridesAndOffsets_;

};


} // /namespace LLGL


#endif



// ================================================================================
