/*
 * MTDevice.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTDevice.h"
#include <LLGL/Platform/Platform.h>


namespace LLGL
{


NSUInteger MTDevice::FindSuitableSampleCount(id<MTLDevice> device, NSUInteger samples)
{
    while (samples > 1u)
    {
        if ([device supportsTextureSampleCount:samples])
            return samples;
        --samples;
    }
    return 4u; // Supported by all macOS and iOS devices; 1 is not supported according to Metal validation layer
}

NSUInteger MTDevice::FindSuitableSampleCountOr1(id<MTLDevice> device, NSUInteger samples)
{
    if (samples > 1u)
        return MTDevice::FindSuitableSampleCount(device, samples);
    else
        return 1u;
}

bool MTDevice::SupportsDeferredStoreActions(id<MTLDevice> device)
{
    #if LLGL_OS_IOS
    if (@available(iOS 13.0, *))
        return [device supportsFamily:MTLGPUFamilyApple2];
    return [device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v1];
    #else
    return true;
    #endif
}

bool MTDevice::SupportsStoreAndResolve(id<MTLDevice> device)
{
    #if LLGL_OS_IOS
    if (@available(iOS 13.0, *))
        return [device supportsFamily:MTLGPUFamilyApple3];
    if (@available(iOS 9.0, *))
        return [device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v1];
    return false;
    #else
    return true;
    #endif
}

bool MTDevice::SupportsDepthResolve(id<MTLDevice> device)
{
    #if LLGL_OS_IOS
    return SupportsStoreAndResolve(device);
    #else
    if (@available(macOS 10.15, *))
        return ([device supportsFamily:MTLGPUFamilyApple3] || [device supportsFamily:MTLGPUFamilyMac2]);
    return false;
    #endif
}

bool MTDevice::SupportsIntegerColorResolve(id<MTLDevice> device)
{
    #if LLGL_OS_IOS
    return false;
    #else
    if (@available(macOS 10.15, *))
        return ![device supportsFamily:MTLGPUFamilyApple2];
    return true;
    #endif
}

bool MTDevice::SupportsStencilResolve(id<MTLDevice> device)
{
    if (@available(macOS 10.15, iOS 13.0, *))
        return ([device supportsFamily:MTLGPUFamilyApple5] || [device supportsFamily:MTLGPUFamilyMac2]);
    return false;
}

} // /namespace LLGL



// ================================================================================
