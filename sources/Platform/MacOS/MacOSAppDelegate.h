/*
 * MacOSAppDelegate.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MACOS_APP_DELEGATE_H
#define LLGL_MACOS_APP_DELEGATE_H


namespace LLGL
{


// Allocates the NSApplicationDelegate if not already done.
void LoadNSAppDelegate();

// Releases the autorelease pool to drain all of its autoreleased objects and re-allocates the pool.
void DrainAutoreleasePool();


} // /namespace LLGL


#endif



// ================================================================================
