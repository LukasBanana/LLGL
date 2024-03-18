/*
 * VideoAdapter.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VIDEO_ADAPTER_H
#define LLGL_VIDEO_ADAPTER_H


#include <LLGL/Export.h>
#include <LLGL/DisplayFlags.h>
#include <LLGL/Container/UTF8String.h>
#include "../Core/Vendor.h"
#include <vector>
#include <cstdint>


namespace LLGL
{


// Video output structure.
struct VideoAdapterOutputInfo
{
    // List of all display modes for this video output.
    std::vector<DisplayMode> displayModes;
};

// Simple structure with meta data about a video adapter.
struct VideoAdapterInfo
{
    UTF8String                          name;
    DeviceVendor                        vendor      = DeviceVendor::Undefined;
    std::uint64_t                       videoMemory = 0;
    std::vector<VideoAdapterOutputInfo> outputs;
};


} // /namespace LLGL


#endif



// ================================================================================
