/*
 * WGBindGroup.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_BIND_GROUP_H
#define LLGL_WG_BIND_GROUP_H


#include <LLGL/Container/ArrayView.h>
#include <memory>
#include <webgpu/webgpu.h>


namespace LLGL
{


// This is a simple wrapper for WGPUBindGroup instances.
class WGBindGroup
{

    public:

        WGBindGroup(WGPUDevice device, WGPUBindGroupLayout bindGroupLayout, ArrayView<WGPUBindGroupEntry> entries);
        ~WGBindGroup();

        // Returns the native WebGPU bind group.
        inline WGPUBindGroup GetNative() const
        {
            return bindGroup_;
        }

    private:

        WGPUBindGroup bindGroup_ = nullptr;

};

using WGBindGroupPtr = std::unique_ptr<WGBindGroup>;


} // /namespace LLGL


#endif



// ================================================================================
