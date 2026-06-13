/*
 * XRConfiguration.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_XR_CONFIGURATION_H
#define LLGL_XR_CONFIGURATION_H

#include <LLGL/Container/ArrayView.h>


namespace LLGL
{


/** \brief Configuration structure for OpenXR-specific settings.  */
struct XRConfigurationOpenXR
{
    /**
    \brief List of OpenXR instance extensions to enable when loading the XR system.
    The ones that are not supported, will be ignored.
    */
    ArrayView<const char*> instanceExtensions;
};


} // namespace LLGL


#endif



// ================================================================================
