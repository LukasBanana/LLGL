/*
 * WGBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_BUFFER_H
#define LLGL_WG_BUFFER_H


#include <LLGL/Buffer.h>
#include <webgpu/webgpu.h>


namespace LLGL
{


class WGBuffer : public Buffer
{

    public:

        #include <LLGL/Backend/Buffer.inl>

    public:

        WGBuffer(WGPUDevice device, const BufferDescriptor& bufferDesc);
        ~WGBuffer();

        // Returns the native WebGPU buffer.
        inline WGPUBuffer GetNative() const
        {
            return buffer_;
        }

        // Returns the total size (in bytes) of the native WebGPU buffer.
        inline std::uint64_t GetSize() const
        {
            return size_;
        }

    private:

        WGPUBuffer      buffer_ = nullptr;
        std::uint64_t   size_   = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
