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
#include <LLGL/Deprecated.h>


namespace LLGL
{


/* ----- Structures ----- */

/**
\brief Display mode structure.
\remarks Contains the properties of a display such as resolution and refresh rate.
\see Display::SetDisplayMode
*/
struct DisplayMode
{
    //! Display resolution (in pixels).
    Extent2D        resolution;

    //! Display refresh rate (in Hz).
    std::uint32_t   refreshRate = 0;
};

//! \deprecated Since 0.04b; Use LLGL::DisplayMode instead!
LLGL_DEPRECATED("LLGL::DisplayModeDescriptor is deprecated since 0.04b; Use LLGL::DisplayMode instead!", "DisplayMode") 
typedef DisplayMode DisplayModeDescriptor;


/* ----- Operators ----- */

//! Compares the two specified display mode descriptors on equality.
LLGL_EXPORT bool operator == (const DisplayMode& lhs, const DisplayMode& rhs);

//! Compares the two specified display mode descriptors on inequality.
LLGL_EXPORT bool operator != (const DisplayMode& lhs, const DisplayMode& rhs);


/* ----- Functions ----- */

/**
\brief Compares the two display modes in a strict-weak-order (SWO) fashion.
\ingroup group_compare_swo
*/
LLGL_EXPORT bool CompareSWO(const DisplayMode& lhs, const DisplayMode& rhs);

/**
\brief Returns the ratio of the specified extent as another extent, i.e. all attributes are divided by their greatest common divisor.
\remarks This can be used to print out a display mode resolution in a better format (e.g. "16:9" rather than "1920:1080").
\see DisplayMode::resolution
*/
LLGL_EXPORT Extent2D GetExtentRatio(const Extent2D& extent);


} // /namespace LLGL


#endif



// ================================================================================
