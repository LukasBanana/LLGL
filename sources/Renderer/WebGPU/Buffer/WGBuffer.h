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


class WGBuffer final : public Buffer
{

    public:

        #include <LLGL/Backend/Buffer.inl>

    public:

        WGBuffer(WGPUDevice device, const BufferDescriptor& bufferDesc);

        // Returns the native WebGPU buffer.
        inline WGPUBuffer GetNative() const
        {
            return buffer_;
        }

    private:

        WGPUBuffer buffer_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
