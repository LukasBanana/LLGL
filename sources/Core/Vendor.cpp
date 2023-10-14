/*
 * Vendor.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Vendor.h"
#include <LLGL/RenderSystemFlags.h>


namespace LLGL
{


// see https://pcisig.com/membership/member-companies
LLGL_EXPORT DeviceVendor GetVendorByID(std::uint16_t id)
{
    switch (id)
    {
        case 0x106B:    return DeviceVendor::Apple;
        case 0x1022:    return DeviceVendor::AMD;
        case 0x8086:    return DeviceVendor::Intel;
        case 0x102B:    return DeviceVendor::Matrox;
        case 0x1414:    return DeviceVendor::Microsoft;
        case 0x10DE:    return DeviceVendor::NVIDIA;
        case 0x108E:    return DeviceVendor::Oracle;
        case 0x15AD:    return DeviceVendor::VMware;
        default:        return DeviceVendor::Undefined;
    }
}

LLGL_EXPORT const char* GetVendorName(DeviceVendor vendor)
{
    switch (vendor)
    {
        case DeviceVendor::Apple:       return "Apple Inc.";
        case DeviceVendor::AMD:         return "Advanced Micro Devices, Inc.";
        case DeviceVendor::Intel:       return "Intel Corporation";
        case DeviceVendor::Matrox:      return "Matrox Electronic Systems Ltd.";
        case DeviceVendor::Microsoft:   return "Microsoft Corporation";
        case DeviceVendor::NVIDIA:      return "NVIDIA Corporation";
        case DeviceVendor::Oracle:      return "Oracle Corporation";
        case DeviceVendor::VMware:      return "VMware Inc.";
        default:                        return "";
    }
}

LLGL_EXPORT bool MatchPreferredVendor(DeviceVendor vendor, long renderSystemFlags)
{
    switch (vendor)
    {
        case DeviceVendor::NVIDIA:
            return ((renderSystemFlags & RenderSystemFlags::PreferNVIDIA) != 0);

        case DeviceVendor::AMD:
            return ((renderSystemFlags & RenderSystemFlags::PreferAMD) != 0);

        case DeviceVendor::Intel:
            return ((renderSystemFlags & RenderSystemFlags::PreferIntel) != 0);

        default:
            return false;
    }
}


} // /namespace LLGL



// ================================================================================
