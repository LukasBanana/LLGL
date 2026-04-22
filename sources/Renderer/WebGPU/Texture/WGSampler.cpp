/*
 * WGSampler.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGSampler.h"
#include "../WGCore.h"
#include "../WGTypes.h"
#include "../../../Core/Assertion.h"


namespace LLGL
{


WGSampler::WGSampler(WGPUDevice device, const SamplerDescriptor& desc)
{
    WGPUSamplerDescriptor wgpuSamplerDesc;
    {
        wgpuSamplerDesc.nextInChain     = nullptr;
        wgpuSamplerDesc.label           = ToWGStringView(desc.debugName);
        wgpuSamplerDesc.addressModeU    = WGTypes::ToWGAddressMode(desc.addressModeU);
        wgpuSamplerDesc.addressModeV    = WGTypes::ToWGAddressMode(desc.addressModeV);
        wgpuSamplerDesc.addressModeW    = WGTypes::ToWGAddressMode(desc.addressModeW);
        wgpuSamplerDesc.magFilter       = WGTypes::ToWGFilterMode(desc.magFilter);
        wgpuSamplerDesc.minFilter       = WGTypes::ToWGFilterMode(desc.minFilter);
        wgpuSamplerDesc.mipmapFilter    = WGTypes::ToWGMipmapFilterMode(desc.mipMapFilter);
        wgpuSamplerDesc.lodMinClamp     = desc.minLOD;
        wgpuSamplerDesc.lodMaxClamp     = desc.maxLOD;
        wgpuSamplerDesc.compare         = (desc.compareEnabled ? WGTypes::ToWGCompareFunc(desc.compareOp) : WGPUCompareFunction_Undefined);
        wgpuSamplerDesc.maxAnisotropy   = static_cast<std::uint16_t>(desc.maxAnisotropy);
    }
    sampler_ = wgpuDeviceCreateSampler(device, &wgpuSamplerDesc);
    LLGL_ASSERT_PTR(sampler_);
}

WGSampler::~WGSampler()
{
    wgpuSamplerRelease(sampler_);
}

bool WGSampler::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}


} // /namespace LLGL



// ================================================================================
