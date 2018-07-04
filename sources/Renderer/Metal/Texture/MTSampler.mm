/*
 * MTSampler.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTSampler.h"
#include "../MTTypes.h"


namespace LLGL
{


static MTLSamplerBorderColor GetBorderColor(const ColorRGBAf& color)
{
    if (color.r == 0.0f && color.g == 0.0f && color.b == 0.0f)
    {
        if (color.a == 0.0f)
            return MTLSamplerBorderColorTransparentBlack;
        else
            return MTLSamplerBorderColorOpaqueBlack;
    }
    return MTLSamplerBorderColorOpaqueWhite;
}

static void Convert(MTLSamplerDescriptor* dst, const SamplerDescriptor& src)
{
    dst.sAddressMode    = MTTypes::ToMTLSamplerAddressMode(src.addressModeU);
    dst.tAddressMode    = MTTypes::ToMTLSamplerAddressMode(src.addressModeV);
    dst.rAddressMode    = MTTypes::ToMTLSamplerAddressMode(src.addressModeW);
    dst.minFilter       = MTTypes::ToMTLSamplerMinMagFilter(src.minFilter);
    dst.magFilter       = MTTypes::ToMTLSamplerMinMagFilter(src.magFilter);
    dst.mipFilter       = (src.mipMapping ? MTTypes::ToMTLSamplerMipFilter(src.mipMapFilter) : MTLSamplerMipFilterNotMipmapped);
    //TODO: src.mipMapLODBias;
    dst.lodMinClamp     = src.minLOD;
    dst.lodMaxClamp     = src.maxLOD;
    dst.maxAnisotropy   = static_cast<NSUInteger>(src.maxAnisotropy);
    dst.compareFunction = (src.compareEnabled ? MTTypes::ToMTLCompareFunction(src.compareOp) : MTLCompareFunctionNever);
    dst.borderColor     = GetBorderColor(src.borderColor);
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
