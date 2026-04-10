/*
 * WGBufferArray.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_BUFFER_ARRAY_H
#define LLGL_WG_BUFFER_ARRAY_H


#include <LLGL/Buffer.h>
#include <LLGL/BufferArray.h>
#include <webgpu/webgpu.h>
#include <vector>


namespace LLGL
{


class WGBufferArray final : public BufferArray
{

    public:

        WGBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray);

        inline const std::vector<WGPUBuffer>& GetNativeBuffers() const
        {
            return wgpuBuffers_;
        }

    private:

        std::vector<WGPUBuffer> wgpuBuffers_;

};


} // /namespace LLGL


#endif



// ================================================================================
