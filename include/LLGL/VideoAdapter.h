/*
 * VideoAdapter.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VIDEO_ADAPTER_H
#define LLGL_VIDEO_ADAPTER_H


#include "Export.h"
#include "DisplayFlags.h"

#include <vector>
#include <string>
#include <cstdint>


namespace LLGL
{


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
