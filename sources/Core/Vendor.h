/*
 * Vendor.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VENDOR_H
#define LLGL_VENDOR_H


#include <LLGL/Export.h>
#include <cstdint>


namespace LLGL
{


// GPU vendor ID enumeration.
enum class DeviceVendor
{
    Undefined,

    Apple,
    AMD,
    Intel,
    Matrox,
    Microsoft,
    NVIDIA,
    Oracle,
    VMware,
};


// Returns the device vendor by the specified ID number.
LLGL_EXPORT DeviceVendor GetVendorByID(std::uint16_t id);

// Returns the name of the hardware vendor by the specified ID number.
LLGL_EXPORT const char* GetVendorName(DeviceVendor vendor);

// Returns true if the vendor ID and the render system flags for a preferred vendor match.
LLGL_EXPORT bool MatchPreferredVendor(DeviceVendor vendor, long renderSystemFlags);


} // /namespace LLGL


#endif



// ================================================================================
