/*
 * MTSampler.mm
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTSampler.h"
#include "../MTTypes.h"
#include "../../ResourceUtils.h"
#include <algorithm>
#include <LLGL/Platform/Platform.h>


namespace LLGL
{


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

static void Convert(MTLSamplerDescriptor* dst, const SamplerDescriptor& src)
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

MTSampler::MTSampler(id<MTLDevice> device, const SamplerDescriptor& desc)
{
    MTLSamplerDescriptor* samplerDesc = [[MTLSamplerDescriptor alloc] init];
    Convert(samplerDesc, desc);
    native_ = [device newSamplerStateWithDescriptor:samplerDesc];
    [samplerDesc release];
}

MTSampler::~MTSampler()
{
    [native_ release];
}


} // /namespace LLGL



// ================================================================================
