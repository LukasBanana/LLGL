/*
 * VideoAdapter.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VIDEO_ADAPTER_H
#define LLGL_VIDEO_ADAPTER_H


#include "Export.h"
#include "Types.h"
#include <vector>
#include <string>
#include <cstdint>


namespace LLGL
{


/* ----- Structures ----- */

/**
\brief Display mode descriptor structure.
\remarks Describes the attributes of a physical display.
The counterpart for a virtual video mode is the VideoModeDescriptor structure.
\see VideoOutputDescriptor::displayModes
\see VideoModeDescriptor
*/
struct DisplayModeDescriptor
{
    //! Display resolution (in pixels).
    Extent2D        resolution;

    //! Display refresh rate (in Hz).
    std::uint32_t   refreshRate = 0;
};

/**
\brief Video output structure.
\see VideoAdapterDescriptor::outputs
*/
struct VideoOutputDescriptor
{
    //! List of all display mode descriptors for this video output.
    std::vector<DisplayModeDescriptor> displayModes;
};

//! Video adapter descriptor structure.
struct VideoAdapterDescriptor
{
    //! Hardware adapter name (name of the GPU).
    std::wstring                        name;

    //! Vendor name (e.g. "NVIDIA Corporation", "Advanced Micro Devices, Inc." etc.).
    std::string                         vendor;

    //! Video memory size (in bytes).
    std::uint64_t                       videoMemory = 0;

    //! List of all adapter output descriptors.
    std::vector<VideoOutputDescriptor>  outputs;
};


/* ----- Operators ----- */

LLGL_EXPORT bool operator == (const DisplayModeDescriptor& lhs, const DisplayModeDescriptor& rhs);
LLGL_EXPORT bool operator == (const DisplayModeDescriptor& lhs, const DisplayModeDescriptor& rhs);


/* ----- Functions ----- */

//! Compares the two display modes in a strict-weak-order (SWO) fashion.
LLGL_EXPORT bool CompareSWO(const DisplayModeDescriptor& lhs, const DisplayModeDescriptor& rhs);


} // /namespace LLGL


#endif



// ================================================================================
