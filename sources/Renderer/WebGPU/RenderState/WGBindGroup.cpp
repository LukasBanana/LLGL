/*
 * WGBindGroup.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGBindGroup.h"
#include "../WGCore.h"


namespace LLGL
{


WGBindGroup::WGBindGroup(WGPUDevice device, WGPUBindGroupLayout bindGroupLayout, ArrayView<WGPUBindGroupEntry> entries)
{
    WGPUBindGroupDescriptor bindGroupDesc;
    {
        bindGroupDesc.nextInChain   = nullptr;
        bindGroupDesc.label         = WGPU_STRING_VIEW_INIT;
        bindGroupDesc.layout        = bindGroupLayout;
        bindGroupDesc.entryCount    = entries.size();
        bindGroupDesc.entries       = entries.data();
    }
    bindGroup_ = wgpuDeviceCreateBindGroup(device, &bindGroupDesc);
    WGThrowIfCreateFailed(bindGroup_, "WGPUBindGroup");
}

WGBindGroup::~WGBindGroup()
{
    wgpuBindGroupRelease(bindGroup_);
}


} // /namespace LLGL



// ================================================================================
