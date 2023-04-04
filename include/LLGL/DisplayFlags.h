/*
 * DisplayFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DISPLAY_FLAGS_H
#define LLGL_DISPLAY_FLAGS_H


#include <LLGL/Export.h>
#include <LLGL/Types.h>


namespace LLGL
{


/* ----- Structures ----- */

/**
\brief Display mode descriptor structure.
\remarks Describes the attributes of a physical display.
\see VideoOutputDescriptor::displayModes
\see SwapChainDescriptor
*/
struct DisplayModeDescriptor
{
    //! Display resolution (in pixels).
    Extent2D        resolution;

    //! Display refresh rate (in Hz).
    std::uint32_t   refreshRate = 0;
};


/* ----- Operators ----- */

//! Compares the two specified display mode descriptors on equality.
LLGL_EXPORT bool operator == (const DisplayModeDescriptor& lhs, const DisplayModeDescriptor& rhs);

//! Compares the two specified display mode descriptors on inequality.
LLGL_EXPORT bool operator != (const DisplayModeDescriptor& lhs, const DisplayModeDescriptor& rhs);


/* ----- Functions ----- */

/**
\brief Compares the two display modes in a strict-weak-order (SWO) fashion.
\ingroup group_compare_swo
*/
LLGL_EXPORT bool CompareSWO(const DisplayModeDescriptor& lhs, const DisplayModeDescriptor& rhs);

/**
\brief Returns the ratio of the specified extent as another extent, i.e. all attributes are divided by their greatest common divisor.
\remarks This can be used to print out a display mode resolution in a better format (e.g. "16:9" rather than "1920:1080").
\see DisplayModeDescriptor::resolution
*/
LLGL_EXPORT Extent2D GetExtentRatio(const Extent2D& extent);


} // /namespace LLGL


#endif



// ================================================================================
