/*
 * VideoAdapter.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VIDEO_ADAPTER_DEPRECATED_H
#define LLGL_VIDEO_ADAPTER_DEPRECATED_H


#include <LLGL/Deprecated.h>
#include <LLGL/Export.h>
#include <LLGL/DisplayFlags.h>
#include <vector>
#include <string>
#include <cstdint>


namespace LLGL
{


/* ----- Structures ----- */

//! \deprecated Since 0.04b; Write custom structure instead!
struct LLGL_DEPRECATED("LLGL::VideoOutputDescriptor is deprecated since 0.04b; Write a custom structure instead!") VideoOutputDescriptor
{
    std::vector<DisplayMode> displayModes;
};

//! \deprecated Since 0.04b; Write custom structure instead!
struct LLGL_DEPRECATED("LLGL::VideoAdapterDescriptor is deprecated since 0.04b; Write a custom structure instead!") VideoAdapterDescriptor
{
    std::wstring                        name;
    std::string                         vendor;
    std::uint64_t                       videoMemory = 0;
    std::vector<VideoOutputDescriptor>  outputs;
};


} // /namespace LLGL


#endif



// ================================================================================
