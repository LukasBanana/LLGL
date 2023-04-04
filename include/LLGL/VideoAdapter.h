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
#include <vector>
#include <string>
#include <cstdint>


namespace LLGL
{


//TODO: this header is currently only included by internal classes

/* ----- Structures ----- */

/**
\brief Video output structure.
\see VideoAdapterDescriptor::outputs
\todo Currently unused in the interface.
*/
struct VideoOutputDescriptor
{
    //! List of all display mode descriptors for this video output.
    std::vector<DisplayModeDescriptor> displayModes;
};

/**
\brief Video adapter descriptor structure.
\remarks A video adapter determines the output capabilities of a GPU.
\todo Currently unused in the interface.
*/
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


} // /namespace LLGL


#endif



// ================================================================================
