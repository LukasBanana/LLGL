/*
 * VideoAdapter.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VIDEO_ADAPTER_H
#define LLGL_VIDEO_ADAPTER_H


#include "Export.h"
#include <vector>
#include <string>


namespace LLGL
{


/* ----- Structures ----- */

//! Video display mode structure.
struct VideoDisplayMode
{
    unsigned int width          = 0; //!< Display resolution width (in pixels).
    unsigned int height         = 0; //!< Display resolution height (in pixels).
    unsigned int refreshRate    = 0; //!< Refresh reate (in Hz).
};

//! Video output structure.
struct VideoOutput
{
    std::vector<VideoDisplayMode> displayModes; //!< Video display mode list.
};

//! Video adapter descriptor structure.
struct VideoAdapterDescriptor
{
    std::wstring                name;               //!< Hardware adapter name (name of the GPU).
    std::string                 vendor;             //!< Vendor name.
    unsigned long long          videoMemory = 0;    //!< Video memory size (in bytes).
    std::vector<VideoOutput>    outputs;            //!< Adapter outputs.
};


/* ----- Operators ----- */

LLGL_EXPORT bool operator == (const VideoDisplayMode& lhs, const VideoDisplayMode& rhs);
LLGL_EXPORT bool operator == (const VideoDisplayMode& lhs, const VideoDisplayMode& rhs);


/* ----- Functions ----- */

//! Compares the two video display modes in a strict-weak-order (SWO) fashion.
LLGL_EXPORT bool CompareSWO(const VideoDisplayMode& lhs, const VideoDisplayMode& rhs);


} // /namespace LLGL


#endif



// ================================================================================
