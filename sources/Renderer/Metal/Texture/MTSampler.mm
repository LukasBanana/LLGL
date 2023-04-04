/*
 * MTSampler.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTSampler.h"
#include "../MTTypes.h"
#include "../../ResourceUtils.h"
#include <algorithm>
#include <LLGL/Platform/Platform.h>


namespace LLGL
{


MTSampler::MTSampler(id<MTLDevice> device, const SamplerDescriptor& desc) :
    native_ { MTSampler::CreateNative(device, desc) }
{
}

MTSampler::~MTSampler()
{
    [native_ release];
}

#ifndef LLGL_OS_IOS

static MTLSamplerBorderColor GetBorderColor(const float (&color)[4])
{
    switch (GetStaticSamplerBorderColor(color))
    {
        default:
        case StaticSamplerBorderColor::TransparentBlack:    return MTLSamplerBorderColorTransparentBlack;
        case StaticSamplerBorderColor::OpaqueBlack:         return MTLSamplerBorderColorOpaqueBlack;
        case StaticSamplerBorderColor::OpaqueWhite:         return MTLSamplerBorderColorOpaqueWhite;
    }
}

#endif // /LLGL_OS_IOS

void MTSampler::ConvertDesc(MTLSamplerDescriptor* dst, const SamplerDescriptor& src)
{
    dst.sAddressMode    = MTTypes::ToMTLSamplerAddressMode(src.addressModeU);
    dst.tAddressMode    = MTTypes::ToMTLSamplerAddressMode(src.addressModeV);
    dst.rAddressMode    = MTTypes::ToMTLSamplerAddressMode(src.addressModeW);
    dst.minFilter       = MTTypes::ToMTLSamplerMinMagFilter(src.minFilter);
    dst.magFilter       = MTTypes::ToMTLSamplerMinMagFilter(src.magFilter);
    dst.mipFilter       = (src.mipMapEnabled ? MTTypes::ToMTLSamplerMipFilter(src.mipMapFilter) : MTLSamplerMipFilterNotMipmapped);
    //TODO: src.mipMapLODBias;
    dst.lodMinClamp     = src.minLOD;
    dst.lodMaxClamp     = src.maxLOD;
    dst.maxAnisotropy   = std::max<NSUInteger>(1, std::min<NSUInteger>(static_cast<NSUInteger>(src.maxAnisotropy), 16));
    dst.compareFunction = (src.compareEnabled ? MTTypes::ToMTLCompareFunction(src.compareOp) : MTLCompareFunctionNever);
    #ifndef LLGL_OS_IOS
    dst.borderColor     = GetBorderColor(src.borderColor);
    #endif // /LLGL_OS_IOS
}

id<MTLSamplerState> MTSampler::CreateNative(id<MTLDevice> device, const SamplerDescriptor& desc)
{
    MTLSamplerDescriptor* samplerStateDesc = [[MTLSamplerDescriptor alloc] init];
    MTSampler::ConvertDesc(samplerStateDesc, desc);
    id<MTLSamplerState> samplerState = [device newSamplerStateWithDescriptor:samplerStateDesc];
    [samplerStateDesc release];
    return samplerState;
}


} // /namespace LLGL



// ================================================================================
