/*
 * PlatformUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_PLATFORM_UTILS_H
#define LLGL_PLATFORM_UTILS_H


#include <LLGL/Export.h>


namespace LLGL
{


// Returns true if the current process is running on a mobile device.
// For WebAssembly, this needs to be dynamic as it cannot be known at compile-time where this will run.
LLGL_EXPORT bool IsRunningOnMobileDevice();


} // /namespace LLGL


#endif



// ================================================================================
